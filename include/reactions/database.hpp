/*! \file
  \brief Define the base database class
 */
#pragma once
#include <fstream>
#include <ios>
#include <limits>
#include <tuple>
#include <vector>

#include "reactions/exceptions.hpp"
#include "reactions/fields.hpp"

/// Common tools and base objects to handle databases
namespace reactions::database {

  /*! \brief Base database class
   *
   */
  template <class Element, class NameField, class IdField> class database {

  public:
    using element_type = Element;

    virtual ~database() {}

    /*! \brief All the elements in the database file
     *
     * Calling this function does not alter the cache. If the cache is
     * enabled a copy of its content is returned. If not, elements are
     * read from the database file.
     *
     * This function returns the elements in the database and those registered
     * by the user.
     */
    std::vector<element_type> all_elements() const {

      std::vector<element_type> out;

      switch (m_cache.status()) {

      case (cache::full):

        out.insert(out.end(), m_cache.begin(), m_cache.end());
        return out;

      default:

        // open the database to count the number of lines
        auto file = open_database();

        auto start = skip_commented_lines(file);

        std::size_t count = 0;
        while (file.ignore(element_type::line_size + 1)) // include end-of-line
          ++count;

        file.clear(); // we reached the end of the file

        // go back to the start of the table and read the elements
        file.seekg(start);

        out.reserve(count + m_cache.size());

        for (auto i = 0u; i < count; ++i) {
          std::string line;
          std::getline(file, line);
          out.emplace_back(read_element(line));
        }

        if (m_cache.size())
          out.insert(out.end(), m_cache.begin(), m_cache.end());

        file.close();
      }

      return out;
    }

    /// Clear the cache, removing also user-registered elements
    void clear_cache() { m_cache.clear(); }

    /// Disable the cache
    void disable_cache() { m_cache.clear_database_elements(); }

    /* \brief Enable the internal cache.
     *
     * All the values in the database will be read an stored in a internal
     * cache. This will speed-up the code, with the caveat of consuming some
     * memory.
     */
    void enable_cache() {

      if (m_cache.status() == cache::full)
        return;

      // open the database to count the number of lines
      auto file = open_database();

      auto start = skip_commented_lines(file);

      std::size_t count = 0;
      while (file.ignore(element_type::line_size + 1)) // include end-of-line
        ++count;

      file.clear(); // we reached the end of the file

      // go back to the start of the table and read the elements
      file.seekg(start);

      std::string line;
      m_cache.add_database_elements(count,
                                    [this, &file, &line]() -> element_type {
                                      std::getline(file, line);
                                      return read_element(line);
                                    });

      file.close();
    }

    /// Get the path to the database file
    std::string const &get_database_path() const { return m_db; }

    /*! \brief Register a new element
     *
     * The new element must have a name and a ID that does not clash with
     * any of the database used.
     *
     */
    template <class... Args> void register_element(Args &&...args) {

      element_type new_element{std::forward<Args>(args)...};

      // If the cache is enabled, the checks are done within the cache object,
      // otherwise we must check the database
      if (m_cache.status() != cache::full) {

        auto file = open_database();

        skip_commented_lines(file);

        std::string line;

        while (std::getline(file, line)) {

          typename NameField::value_type name;
          if (reactions::fields::read_field<typename NameField::range_type>(
                  name, line) == reactions::fields::failed)
            throw reactions::database_error(
                "Error reading the database; data format not understood");

          if (new_element.template get<NameField>() == name)
            throw reactions::database_error(
                "Attempt to register an element with similar name to an "
                "element in the database");

          typename IdField::value_type id;
          if (reactions::fields::read_field<typename IdField::range_type>(
                  id, line) == reactions::fields::failed)
            throw reactions::database_error(
                "Error reading the database; data format not understood");

          if (new_element.template get<IdField>() == id)
            throw reactions::database_error(
                "Attempt to register an element with similar ID to an "
                "element in the database");
        }
      }

      // this must be done after the checks are done to prevent leaving
      // the cache in an invalid state
      m_cache.add_user_element(std::move(new_element));
    }

    /* \brief Set the path to the database file.
     *
     * If the cache is enabled, reloads the content using the new path.
     */
    void set_database_path(std::string const &s) {
      m_db = s;
      if (m_cache.status() == cache::full) {
        disable_cache();
        enable_cache();
      }
    }

    /// Create an element accessing by ID
    virtual element_type operator()(int id) const = 0;

    /// Create an element accessing by name
    virtual element_type operator()(std::string const &str) const = 0;

  protected:
    /// Path to the database file
    std::string m_db;

    /// \brief Cache of elements
    class cache {

    public:
      /// Code to define the status of the cache
      enum cache_status { empty, user, full };

      using cache_type = std::vector<element_type>;
      using const_iterator_type = typename cache_type::const_iterator;
      using size_type = typename cache_type::size_type;

      cache() = default;

      /// Clear the cache
      void clear() {
        m_vector.clear();
        m_vector.shrink_to_fit();
        m_separator = 0;
      }

      /// Clear the cache
      void clear_database_elements() {
        m_vector.erase(database_cbegin(), database_cend());
        m_vector.shrink_to_fit();
        m_separator = 0;
      }

      /// Status of the cache
      cache_status status() const {
        if (m_vector.size()) {
          if (database_size() == 0)
            return cache_status::user;
          else
            return cache_status::full;
        } else
          return cache_status::empty;
      }

      /// Underlying vector of elements
      cache_type const &elements() const { return m_vector; }

      /// Underlying vector of elements
      cache_type &elements() { return m_vector; }

      /// Begining of the cache
      const_iterator_type begin() const { return m_vector.cbegin(); }

      /// End of the cache
      const_iterator_type end() const { return m_vector.cend(); }

      /// Begining of the database elements
      const_iterator_type database_cbegin() const { return m_vector.cbegin(); }

      /// End of the database elements
      const_iterator_type database_cend() const {
        return m_vector.cbegin() + m_separator;
      }

      /// Begining of the user-registered elements
      const_iterator_type user_registered_cbegin() const {
        return database_cend();
      }

      /// End of the user-registered elements
      const_iterator_type user_registered_cend() const {
        return m_vector.cend();
      }

      /// Number of elements associated to the database
      size_type database_size() const { return m_separator; }

      /// Number of user-registered elements
      size_type user_registered_size() const {
        return m_vector.size() - m_separator;
      }

      /// Number of cached elements
      size_type size() const { return m_vector.size(); }

      /// Add elements from a database by calling the given function several
      /// times
      template <class ElementReader>
      void add_database_elements(size_type n, ElementReader func) {

        cache_type new_cache;
        new_cache.reserve(n + user_registered_size());

        for (auto i = 0u; i < n; ++i) {

          auto new_element = func();

          // check that we do not repeat any entry
          if (user_registered_size() != 0) {
            auto cend = user_registered_cend();
            if (std::find_if(user_registered_cbegin(), cend,
                             [&new_element](element_type const &el) {
                               return (
                                   el.template get<NameField>() ==
                                       new_element.template get<NameField>() ||
                                   el.template get<IdField>() ==
                                       new_element.template get<IdField>());
                             }) != cend)
              throw reactions::database_error(
                  (std::string{"User-defined element clashes with database "
                               "element: \""} +
                   new_cache.back().name() + "\"")
                      .c_str());
          }

          new_cache.emplace_back(std::move(new_element));
        }

        // insert the elements and assign the separator to the number of
        // database elements
        new_cache.insert(new_cache.end(),
                         std::make_move_iterator(user_registered_cbegin()),
                         std::make_move_iterator(user_registered_cend()));
        m_separator = n;
        m_vector = std::move(new_cache);
      }

      /// Add a new element (by the user)
      template <class... Args>
      element_type const &add_user_element(Args &&...args) {
        element_type new_element{std::forward<Args>(args)...};
        auto e = end();
        if (std::find_if(begin(), e,
                         [this, &new_element](element_type const &el) {
                           return (el.template get<NameField>() ==
                                       new_element.template get<NameField>() ||
                                   el.template get<IdField>() ==
                                       new_element.template get<IdField>());
                         }) != e) {
          throw reactions::database_error(
              (std::string{"User-registered element clashes: \""} +
               new_element.name() + "\"")
                  .c_str());
        }
        m_vector.emplace_back(std::move(new_element));
        return m_vector.back();
      }

    protected:
      /// Collection of elements
      cache_type m_vector;

      /// Index that defines elements registered by the user and those read from
      /// the database
      size_type m_separator = 0;

    } m_cache; /// Cache for elements loaded from the database or registered by
               /// the user

    /// Open the database
    std::ifstream open_database() const {

      if (m_db.empty())
        throw reactions::database_error("The database has not been specified");

      std::ifstream file;

      try {
        file.open(m_db);
      } catch (...) {
        throw reactions::database_error("Unable to access the database");
      }

      if (!file.is_open())
        throw reactions::database_error("Unable to access the database");

      return file;
    }

    /// Skip lines with comments (preceeded by *)
    std::streampos skip_commented_lines(std::ifstream &file) const {

      char c;
      while (true) {
        file.get(c);
        if (c == '*')
          file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        else
          break;
      }
      file.unget();

      return file.tellg();
    }

    /// Read a field with the given index from a line
    template <std::size_t I>
    bool read_field(typename element_type::base_type &tuple,
                    std::string const &line) const {

      using field = std::tuple_element_t<I, typename element_type::fields_type>;

      if constexpr (fields::is_optional_field_v<field>) {

        typename field::value_type::value_type value;

        auto sc = reactions::fields::read_field<typename field::range_type>(
            value, line);

        if (sc == reactions::fields::conversion_status::success)
          std::get<I>(tuple).emplace(value);

        return sc != reactions::fields::conversion_status::failed;
      } else
        return reactions::fields::read_field<typename field::range_type>(
                   std::get<I>(tuple), line) !=
               reactions::fields::conversion_status::failed;
    }

    /// Read all the fields from a line
    template <std::size_t... I>
    bool read_line(typename element_type::base_type &tuple,
                   std::string const &line, std::index_sequence<I...>) const {
      return (read_field<I>(tuple, line) && ...);
    }

    /// Read all the fields from a line
    bool read_line(typename element_type::base_type &tuple,
                   std::string const &line) const {
      return read_line(
          tuple, line,
          std::make_index_sequence<element_type::number_of_fields>());
    }

    /// Advance to the next element in the file and read it
    element_type read_element(std::string const &line) const {

      typename element_type::base_type tuple;

      if (!read_line(tuple, line))
        throw reactions::database_error(
            "Error reading the database; data format not understood");

      return tuple;
    }

    /// Access an element using the field accessor
    template <class Field, class T> element_type access(T const &v) const {

      switch (m_cache.status()) {
      case (cache::full): // the full database is loaded

        for (auto const &el : m_cache)
          if (el.template get<Field>() == v)
            return el;

        break; // throws an exception

      case (cache::user): // only user-registered entries exist

        for (auto const &el : m_cache)
          if (el.template get<Field>() == v)
            return el;

        [[fallthrough]]; // continue as if we had no cache

      case (cache::empty): // the cache is empty

        auto file = open_database();

        skip_commented_lines(file);

        std::string line;

        while (std::getline(file, line)) {

          typename Field::value_type ref;
          auto sc = reactions::fields::read_field<typename Field::range_type>(
              ref, line);

          if (sc == reactions::fields::failed)
            throw reactions::database_error(
                "Error reading the database; data format not understood");

          if (ref == v) {
            element_type el = read_element(line);
            file.close();
            return el;
          }
        }

        file.close();
      }

      throw reactions::lookup_error(
          (std::string{"Unable to find element with "} + Field::title + " \"" +
           fields::to_string(v) + '"')
              .c_str());
    }

    database(std::string &&db) : m_db{std::move(db)} {}
    database(database &&) = delete;
    database(database const &) = delete;
    void operator=(database &&) = delete;
    void operator=(database const &) = delete;
  };
} // namespace reactions::database

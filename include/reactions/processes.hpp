/*! \file
  \brief Define the nodes of a reaction or a decay, as well as the way to build
  them.
 */
#ifndef REACTIONS_PROCESSES_HPP
#define REACTIONS_PROCESSES_HPP

#include "reactions/element_traits.hpp"
#include "reactions/exceptions.hpp"
#include "reactions/pow_enum.hpp"
#include "reactions/tokens.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace reactions {
  // Forward declarations
  template <class Element> class element_wrapper;
  template <class Element> class reaction;
  template <class Element> class decay;
} // namespace reactions

/*! \brief Processes among elements (reactions and decays)
 */
namespace reactions::processes {

  /// Definitions for internal use only
  namespace detail {
    /// Node types
    REACTIONS_POW_ENUM_WITH_UNKNOWN(node_kind, element, reaction, decay);

    /*! \brief Internal function to process an expression
     *
     * This function takes the current position in the expression, the end
     * of it and three functions:
     *
     * * **fill_element**: fills an element. The function must check whether
     * it needs to fill a component on the left or right side of an arrow. The
     * start of the element is provided to the function.
     * * **fill_expression**: fills an expression. The function must check
     * whether it needs to fill a component on the left or right side of an
     * arrow.
     * * **arrow_switch**: action to execute when an arrow is detected.
     */
    template <class FillElement, class FillExpression, class ArrowSwitch>
    void process_expression(std::string::const_iterator &sit,
                            std::string::const_iterator const &end,
                            FillElement fill_element,
                            FillExpression fill_expression,
                            ArrowSwitch arrow_switch) {

      while (tokens::match_token<tokens::space>(sit))
        ++sit; // remove leading spaces

      if (tokens::match_token<tokens::left_bra>(sit))
        throw exceptions::__syntax_error(
            "Expression starts with another expression", end - sit);

      auto start = sit; // keep track of the beginning of an expression
      while (sit != end) {

        if (tokens::match_token<tokens::space>(sit)) {

          if (sit == start) {
            // remove consecutive spaces
            start = ++sit;
            continue;
          } else {
            // add the new element
            fill_element(start);
            start = (sit += tokens::space::size);
            continue;
          }
        } else if (tokens::match_token<tokens::left_bra>(sit)) {

          if (sit == start) {

            sit += tokens::left_bra::size;

            // begin a new expression
            fill_expression();

            if (!tokens::match_token<tokens::right_bra>(sit))
              throw exceptions::__syntax_error("Expected closing braces",
                                               end - sit);

            start = (sit += tokens::right_bra::size);

            continue;

          } else {
            // add the new element
            fill_element(start);
            start = sit; // end up in the previous conditional
            continue;
          }
        } else if (tokens::match_token<tokens::right_bra>(sit)) {

          if (sit != start)
            fill_element(start);

          break;

        } else if (tokens::match_token<tokens::arrow>(sit)) {

          if (sit != start)
            fill_element(start);

          start = (sit += tokens::arrow::size);

          // switch to the products
          arrow_switch();

          continue;

        } else
          ++sit;
      }

      if (sit != start)
        fill_element(start);
    }
  } // namespace detail

  /*! \brief Base class for types referred to a node
   *
   * This base class avoids the use of void* pointers
   */
  struct node_object {
    node_object() = default;
    node_object(node_object const &) = default;
    node_object(node_object &&) = default;
    node_object &operator=(node_object const &) = default;
    virtual ~node_object() = default;
  };

  /*! \brief Base class for a process node
   *
   * This abstract class allows to check if the underlying class
   * is an element or a composite object.
   */
  template <class Element> class node final {

  public:
    using element_type = element_wrapper<Element>;
    using reaction_type = reaction<Element>;
    using decay_type = decay<Element>;

    /// Constructor from an element
    node(std::unique_ptr<element_type> &&ptr)
        : m_type{std::move(detail::node_kind::element)}, m_ptr{ptr.release()} {}

    /// Constructor from a reaction
    node(std::unique_ptr<reaction_type> &&ptr)
        : m_type{std::move(detail::node_kind::reaction)}, m_ptr{ptr.release()} {
    }

    /// Construction from a decay
    node(std::unique_ptr<decay_type> &&ptr)
        : m_type{std::move(detail::node_kind::decay)}, m_ptr{ptr.release()} {}

    /// Move constructor
    node(node &&other) : m_type{other.m_type}, m_ptr{other.m_ptr} {
      other.m_ptr = nullptr;
    }

    /// Destructor
    ~node() noexcept(false) {

      if (m_ptr) {
        switch (m_type) {
        case (detail::node_kind::element):
          delete ptr_as_element_wrapper();
          return;
        case (detail::node_kind::reaction):
          delete ptr_as_reaction();
          return;
        case (detail::node_kind::decay):
          delete ptr_as_decay();
          return;
        case (detail::node_kind::unknown):
          throw exceptions::internal_error(
              "A node type should always be set (internal error); please "
              "report the bug");
        }
      }
    }

    node() = delete;
    node(node const &) = delete;
    node &operator=(node const &) = delete;

    /// Check if the underlying class is an element
    bool is_element() const { return m_type == detail::node_kind::element; }

    /// Check if the underlying class is a reaction
    bool is_reaction() const { return m_type == detail::node_kind::reaction; }

    /// Check if the underlying class is a decay
    bool is_decay() const { return m_type == detail::node_kind::decay; }

    /// Get the node type
    detail::node_kind type() const { return m_type; }

    /// Get the pointer to the underlying object
    node_object const *object() const { return m_ptr; }

    /// Return the pointer to the underlying object casted to an element
    Element const *ptr_as_element() const {
      return ptr_as_element_wrapper()->operator->();
    }

    /// Return the pointer to the underlying object casted to a reaction
    reaction_type const *ptr_as_reaction() const {
      return static_cast<reaction_type *>(m_ptr);
    }

    /// Return the pointer to the underlying object casted to a decay
    decay_type const *ptr_as_decay() const {
      return static_cast<decay_type *>(m_ptr);
    }

  protected:
    /// Internal method to return the underlying object casted to a wrapped
    /// element
    element_type const *ptr_as_element_wrapper() const {
      return static_cast<element_type *>(m_ptr);
    }

  private:
    /// Node type
    detail::node_kind m_type = detail::node_kind::unknown;

    /// Underlying object
    node_object *m_ptr = nullptr;
  };

  /// Compare nodes
  template <class Element>
  inline bool check_nodes(std::vector<node<Element>> const &first,
                          std::vector<node<Element>> const &second) {

    auto size = first.size();

    if (size != second.size())
      return false;

    // two masks to count the elements that are gone
    std::vector<bool> mask(size, false);

    for (auto i = 0u; i < size; ++i) {

      for (auto j = 0u; j < size; ++j) {

        if (mask[j])
          // already used
          continue;

        if (second[j].type() != first[i].type())
          // different types
          continue;

        switch (first[i].type()) {
        case (detail::node_kind::element):
          if (*(first[i].ptr_as_element()) == *(second[i].ptr_as_element()))
            mask[j] = true;
          break;
        case (detail::node_kind::reaction):
          if (*(first[i].ptr_as_reaction()) == *(second[i].ptr_as_reaction()))
            mask[j] = true;
          break;
        case (detail::node_kind::decay):
          if (*(first[i].ptr_as_decay()) == *(second[i].ptr_as_decay()))
            mask[j] = true;
          break;
        case (detail::node_kind::unknown):
          throw exceptions::internal_error(
              "A node can not be of unknown type (internal error); please "
              "report the bug");
          break;
        }
      }
      // if we reach this point, no match has been found for "first[i]"
      return false;
    }

    // all of them have been matched
    return true;
  }
} // namespace reactions::processes

namespace reactions {

  /** \brief Template for elements of a reaction or decay.
   *
   * This class wraps the template argument type, which becomes accessible
   * through the * and -> operators.
   */
  template <class Element>
  class element_wrapper final : public processes::node_object {

  public:
    /// Constructor given the underlying element
    element_wrapper(Element &&e)
        : processes::node_object{}, m_element{std::move(e)} {}
    /// Default destructor
    ~element_wrapper() override = default;
    /// Copy constructor
    element_wrapper(const element_wrapper &) = default;
    /// Move constructor
    element_wrapper(element_wrapper &&) = default;
    /// Assignment constructor
    element_wrapper &operator=(const element_wrapper &) = default;
    // No empty constructor
    element_wrapper() = delete;
    /// Access the underlying element by reference
    Element const &operator*() const { return m_element; }
    /// Access the underlying element by pointer
    Element const *operator->() const { return &m_element; }

  protected:
    /// Underlying element
    Element m_element;
  };

  // Friend function must be declared before the class which declares the
  // friendship

  template <element_kind K>
  reaction<element_traits::element_t<K>> make_reaction(std::string const &);

  template <class Element>
  reaction<Element> make_reaction_for(std::string const &,
                                      typename reaction<Element>::builder_type);

  template <class Element>
  decay<Element> make_decay_for(std::string const &str,
                                typename decay<Element>::builder_type builder);

  template <element_kind K>
  decay<element_traits::element_t<K>> make_decay(std::string const &);

  /** \brief Description of a process where reactants generate a set of products
   *
   * Nested reactions must be expressed within parenteses.
   */
  template <class Element>
  class reaction final : public processes::node_object {

  public:
    using element_type = element_wrapper<Element>; /// Underlying element type
    using builder_type =
        element_traits::builder_tpl_t<Element>; /// Signature type of an element
                                                /// builder
    using nodes_type =
        std::vector<processes::node<Element>>; /// Collection of elements

    /// Default move constructor
    reaction(reaction &&) = default;
    /// Default copy constructor
    reaction(const reaction &) = default;
    /// Default assignment operator
    reaction &operator=(const reaction &) = default;
    /// Default destructor
    ~reaction() = default;
    // No empty constructor
    reaction() = delete;

    /// Get the reactants of the reaction
    nodes_type const &reactants() const { return m_reactants; }

    /// Get the products of the reaction
    nodes_type const &products() const { return m_products; }

    /// Compare two reactions
    bool operator==(reaction<Element> const &other) const {

      if (m_reactants.size() != other.m_reactants.size() ||
          m_products.size() != other.m_products.size())
        return false;

      return check_nodes(m_reactants, other.m_reactants) &&
             check_nodes(m_products, other.m_products);
    }

    /// Compare two reactions
    bool operator!=(reaction<Element> const &other) const {
      return !(*this == other);
    }

  protected:
    /// Constructor from the string iterators
    reaction(std::string::const_iterator &&begin,
             std::string::const_iterator const &end, builder_type builder)
        : reaction{begin, end, builder} {}

    /// Create a new instance using the protected constructor
    friend std::unique_ptr<reaction>
    std::make_unique<reaction>(std::string::const_iterator &,
                               std::string::const_iterator const &,
                               builder_type);

    /// Create a new instance using the protected constructor
    friend std::shared_ptr<reaction>
    std::make_shared<reaction>(std::string::const_iterator &,
                               std::string::const_iterator const &,
                               builder_type);

    // Must declare it with different type for friendship to work properly
    template <class E>
    friend reaction<E>
    reactions::make_reaction_for(std::string const &,
                                 typename reaction<E>::builder_type);

    template <element_kind K>
    friend reaction<element_traits::element_t<K>>
    reactions::make_reaction(std::string const &);

    /// Constructor from the string iterators
    reaction(std::string::const_iterator &sit,
             std::string::const_iterator const &end, builder_type builder)
        : processes::node_object{} {

      nodes_type *current_set = &m_reactants;

      auto fill_element =
          [&](std::string::const_iterator const &start) -> void {
        current_set->push_back(
            std::make_unique<element_type>(builder(std::string{start, sit})));
      };
      auto fill_reaction = [&]() -> void {
        current_set->push_back(std::make_unique<reaction>(sit, end, builder));
      };
      auto arrow_switch = [&]() -> void {
        if (!m_reactants.size())
          throw exceptions::__syntax_error("Missing reactants", end - sit);

        if (current_set == &m_products)
          throw exceptions::__syntax_error("Duplicated arrow", end - sit);

        current_set = &m_products;
      };

      processes::detail::process_expression(sit, end, fill_element,
                                            fill_reaction, arrow_switch);

      if (!m_reactants.size())
        throw exceptions::__syntax_error("Missing reactants", end - sit);

      if (!m_products.size())
        throw exceptions::__syntax_error("Missing products", end - sit);
    }

    /// Reactants
    nodes_type m_reactants;
    /// Products
    nodes_type m_products;
  };

  /** \brief Description of a process where head particle generate a set of
   * products
   *
   * This can be seen as a special reaction with only one reactant, and where
   * subsequent composite nodes are also decays.
   */
  template <class Element> class decay final : public processes::node_object {

  public:
    using element_type = element_wrapper<Element>;
    using builder_type = element_traits::builder_tpl_t<Element>;
    using nodes_type = std::vector<processes::node<Element>>;

    /// Default move constructor
    decay(decay &&) = default;
    /// Default copy constructor
    decay(const decay &) = default;
    /// Default assignment operator
    decay &operator=(const decay &) = default;
    /// Default destructor
    ~decay() = default;

    decay() = delete;

    /// Get the head particle of the decay
    Element const &head() const { return *(*m_head); }

    /// Get the products of the decay
    nodes_type const &products() const { return m_products; }

    /// Comparison operator
    bool operator==(decay<Element> const &other) const {

      if (m_products.size() != other.m_products.size())
        return false;

      return (*m_head == *other.m_head) &&
             check_nodes(m_products, other.m_products);
    }

    /// Comparison operator
    bool operator!=(decay<Element> const &other) const {
      return !(*this == other);
    }

  protected:
    /// Constructor from the string iterators
    decay(std::string::const_iterator &&begin,
          std::string::const_iterator const &end, builder_type builder)
        : decay{begin, end, builder} {}

    /// Create a new instance using the protected constructor
    friend std::unique_ptr<decay>
    std::make_unique<decay>(std::string::const_iterator &,
                            std::string::const_iterator const &, builder_type);

    /// Create a new instance using the protected constructor
    friend std::shared_ptr<decay>
    std::make_shared<decay>(std::string::const_iterator &,
                            std::string::const_iterator const &, builder_type);

    // Must declare it with different type for friendship to work properly
    template <class T>
    friend decay<T> reactions::make_decay_for(std::string const &,
                                              typename decay<T>::builder_type);

    template <element_kind K>
    friend decay<element_traits::element_t<K>>
    reactions::make_decay(std::string const &);

    /// Constructor from the string iterators
    decay(std::string::const_iterator &sit,
          std::string::const_iterator const &end, builder_type builder)
        : processes::node_object{} {

      bool fill_products = false; // keep track of the elements we are adding

      auto fill_element =
          [&](std::string::const_iterator const &start) -> void {
        if (!m_head) {
          this->m_head.emplace(builder(std::string{start, sit}));
        } else if (fill_products) {
          this->m_products.emplace_back(
              std::make_unique<element_type>(builder(std::string{start, sit})));
        } else
          throw exceptions::__syntax_error("Missing arrow", end - sit);
      };
      auto fill_decay = [&] {
        if (!m_head) {
          throw exceptions::__syntax_error("Missing head", end - sit);
        } else if (fill_products) {
          this->m_products.push_back(std::make_unique<decay>(
              sit += tokens::left_bra::size, end, builder));
        } else
          throw exceptions::__syntax_error("Missing arrow", end - sit);
      };
      auto arrow_switch = [&] {
        if (fill_products)
          throw exceptions::__syntax_error("Duplicated arrow", end - sit);

        else if (!m_head)
          throw exceptions::__syntax_error("Missing head particle", end - sit);

        fill_products = true;
      };

      processes::detail::process_expression(sit, end, fill_element, fill_decay,
                                            arrow_switch);

      if (!m_head)
        throw exceptions::__syntax_error("No elements have been parsed",
                                         end - sit);

      if (!m_products.size())
        throw exceptions::__syntax_error("Expected products", end - sit);
    }

    /// Head particle
    std::optional<element_type> m_head;
    /// Products
    nodes_type m_products;
  };

  /*! \brief Create a new reaction with a custom builder
   *
   * This function uses the template argument as element type, the first
   * argument as the string to parse and the second is the object used to
   * build the elements (the database).
   */
  template <class Element>
  reaction<Element>
  make_reaction_for(std::string const &str,
                    typename reaction<Element>::builder_type builder) {
    try {

      auto sit = str.cbegin();
      auto const send = str.cend();

      reaction<Element> r{sit, send, builder};

      if (sit != send) {
        if (tokens::match_token<tokens::right_bra>(sit))
          throw exceptions::__syntax_error("Mismatching braces", send - sit);
        else
          throw exceptions::__syntax_error("Invalid syntax", send - sit);
      }

      return r;

    } catch (exceptions::__syntax_error &e) {
      throw e.update(str);
    }
  }

  /*! \brief Create a new reaction
   *
   * Use the default constructor for the element type provided as a template
   * argument.
   */
  template <element_kind K>
  reaction<element_traits::element_t<K>> make_reaction(std::string const &str) {
    return make_reaction_for<element_traits::element_t<K>>(
        str, element_traits::builder<K>);
  }

  /*! \brief Create a new decay with a custom builder
   *
   * This function uses the template argument as element type, the first
   * argument as the string to parse and the second is the object used to
   * build the elements (the database).
   */
  template <class Element>
  decay<Element> make_decay_for(std::string const &str,
                                typename decay<Element>::builder_type builder) {
    try {

      auto sit = str.cbegin();
      auto const send = str.cend();

      decay<Element> d{sit, send, builder};

      if (sit != send) {
        if (tokens::match_token<tokens::right_bra>(sit))
          throw exceptions::__syntax_error("Mismatching braces", send - sit);
        else
          throw exceptions::__syntax_error("Invalid syntax", send - sit);
      }

      return d;

    } catch (exceptions::__syntax_error &e) {
      throw e.update(str);
    }
  }

  /*! \brief Create a new decay
   *
   * Use the default constructor for the element type provided as a template
   * argument.
   */
  template <element_kind K>
  decay<element_traits::element_t<K>> make_decay(std::string const &str) {
    return make_decay_for<element_traits::element_t<K>>(
        str, element_traits::builder<K>);
  }
} // namespace reactions

#endif // REACTIONS_PROCESSES_HPP

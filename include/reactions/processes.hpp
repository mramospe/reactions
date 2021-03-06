/*! \file
  \brief Define the nodes of a reaction or a decay, as well as the way to build
  them.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "reactions/element_traits.hpp"
#include "reactions/exceptions.hpp"
#include "reactions/pow_enum.hpp"
#include "reactions/tokens.hpp"

namespace reactions {

  // Forward declarations
  template <class Element> class element_wrapper;
  template <class Element> class reaction;
  template <class Element> class decay;

  /*! \brief Processes among elements (reactions and decays)
   */
  namespace processes {
    /// Node types
    REACTIONS_POW_ENUM_WITH_UNKNOWN(node_type, element, reaction, decay);

    /*! \brief Internal function to process an expression
     *
     * \param sit current position in the processed string
     * \param end end of the string to process
     * \param fill_element fills an element. The function must check whether
     * it needs to fill a component on the left or right side of an arrow. The
     * start of the element is provided to the function.
     * \param fill_expression: fills an expression. The function must check
     * whether it needs to fill a component on the left or right side of an
     * arrow.
     * \param arrow_switch: action to execute when an arrow is detected.
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
        throw reactions::exceptions::__syntax_error(
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
              throw reactions::exceptions::__syntax_error(
                  "Expected closing braces", end - sit);

            start = (sit += tokens::right_bra::size);

            continue;

          } else {
            // add the new element
            fill_element(start);
            start = sit; // end up in the previous conditional
            continue;
          }
        } else if (tokens::match_token<tokens::right_bra>(sit)) {

          if (sit != start) {
            fill_element(start);
            start = sit;
          }

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

    /*! \brief Make a new process (a reaction or a decay)
     *
     * \param str string to parse
     * \param builder function to build the underlying elements
     * \return the new process
     */
    template <class Process>
    Process make_process(std::string const &str,
                         typename Process::builder_type builder) {

      try {

        auto sit = str.cbegin();
        auto const send = str.cend();

        Process p{sit, send, builder};

        if (sit != send) {
          if (tokens::match_token<tokens::right_bra>(sit))
            throw reactions::exceptions::__syntax_error("Mismatching braces",
                                                        send - sit);
          else
            throw reactions::exceptions::__syntax_error("Invalid syntax",
                                                        send - sit);
        }

        return p;

      } catch (reactions::exceptions::__syntax_error &e) {
        throw e.update(str);
      }
    }
  } // namespace processes

  // Forward declarations
  template <class Element> class element_wrapper;
  template <class Element> class reaction;
  template <class Element> class decay;

  /*! \brief Base class for a process node
   *
   * This abstract class allows to check if the underlying class
   * is an element or a chain object.
   */
  template <class Element, template <class> class Chain>
  class node final
      : protected std::variant<element_wrapper<Element>, Chain<Element>> {

  public:
    using base_type = std::variant<element_wrapper<Element>, Chain<Element>>;
    using element_type = element_wrapper<Element>;
    using chain_type = Chain<Element>;

    /// Use the same constructors as \ref std::variant
    using base_type::base_type;

    /// Check if the underlying class is an element
    bool is_element() const {
      return this->type() == processes::node_type::element;
    }

    /// Check if the underlying class is a reaction
    bool is_reaction() const {
      return type() == processes::node_type::reaction;
    }

    /// Check if the underlying class is a decay
    bool is_decay() const {
      return this->type() == processes::node_type::decay;
    }

    /// Get the node type
    processes::node_type type() const {

      if (std::holds_alternative<element_type>(*this))
        return processes::node_type::element;
      else {
        if constexpr (utils::is_template_specialization_v<chain_type, reaction>)
          return processes::node_type::reaction;
        else if constexpr (utils::is_template_specialization_v<chain_type,
                                                               decay>)
          return processes::node_type::decay;
        else
          static_assert(utils::dependent_false_v<decltype(*this)>,
                        "Unexpected internal compile-time error");
      }
    }

    /// Return the pointer to the underlying object casted to an element
    Element const &as_element() const { return *as_element_wrapper(); }

    /// Return the pointer to the underlying object casted to a reaction
    chain_type const &as_chain() const {
      return std::get<Chain<Element>>(*this);
    }

  protected:
    /// Return the underlying object casted to a wrapped
    element_type const &as_element_wrapper() const {
      return std::get<element_type>(*this);
    }
  };

  /// Internal utilities for the \ref reactions::processes namespace
  namespace processes::detail {
    /*! \brief Compare two nodes
     *
     * \param first node to compare
     * \param second node to compare
     * \return whether the two nodes are equal or not (processing
     * whether they are reactions, decays or elements).
     */
    template <class Element, template <class> class Chain>
    inline bool check_nodes(std::vector<node<Element, Chain>> const &first,
                            std::vector<node<Element, Chain>> const &second) {

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
          case (processes::node_type::element):
            if (first[i].as_element() == second[i].as_element())
              mask[j] = true;
            break;
          case (processes::node_type::reaction):
            if (first[i].as_chain() == second[i].as_chain())
              mask[j] = true;
            break;
          case (processes::node_type::decay):
            if (first[i].as_chain() == second[i].as_chain())
              mask[j] = true;
            break;
          case (processes::node_type::unknown_node_type):
            throw reactions::internal_error(
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
  } // namespace processes::detail
} // namespace reactions

namespace reactions {

  /*! \brief Template for elements of a reaction or decay.
   *
   * This class wraps the template argument type, which becomes accessible
   * through the * and -> operators.
   */
  template <class Element> class element_wrapper final {

  public:
    /// Constructor given the underlying element
    element_wrapper(Element &&e) : m_element{std::move(e)} {}
    /// Default destructor
    ~element_wrapper() = default;
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

  /*! \brief Description of a process where reactants generate a set of products
   *
   * Nested reactions must be expressed within parenteses.
   */
  template <class Element> class reaction final {

  public:
    using element_type = element_wrapper<Element>; /// Underlying element type
    using builder_type =
        element_traits::builder_tpl_t<Element>; /// Signature type of an element
                                                /// builder
    using nodes_type =
        std::vector<node<Element, reaction>>; /// Collection of elements

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

    /*! \brief Compare two reactions
     *
     * \param other the reaction to compare with (of the same
     * underlying type).
     * \return whether the two reactions have the same reactants
     * and products or not.
     */
    bool operator==(reaction<Element> const &other) const {

      if (m_reactants.size() != other.m_reactants.size() ||
          m_products.size() != other.m_products.size())
        return false;

      return processes::detail::check_nodes(m_reactants, other.m_reactants) &&
             processes::detail::check_nodes(m_products, other.m_products);
    }

    /// \copydoc reaction<Element>::operator==
    bool operator!=(reaction<Element> const &other) const {
      return !(*this == other);
    }

  protected:
    /*!\brief Constructor from the string iterators
     *
     * \param begin iterator pointing to the beginning of the string.
     * \param end iterator pointing to the end of the string.
     * \param builder function to create the underlying element from
     * a string.
     */
    reaction(std::string::const_iterator &&begin,
             std::string::const_iterator const &end, builder_type builder)
        : reaction{begin, end, builder} {}

    template <class Process>
    friend Process
    reactions::processes::make_process(std::string const &,
                                       typename Process::builder_type);

    /*!\brief Constructor from the string iterators
     *
     * This constructor is called internally when creating reactions that
     * contain other reactions.
     *
     * \param sit iterator pointing to the current read position in the string.
     * \param end iterator pointing to the end of the string.
     * \param builder function to create the underlying element from
     * a string.
     */
    reaction(std::string::const_iterator &sit,
             std::string::const_iterator const &end, builder_type builder) {

      nodes_type *current_set = &m_reactants;

      auto fill_element =
          [&](std::string::const_iterator const &start) -> void {
        current_set->emplace_back(builder(std::string{start, sit}));
      };
      auto fill_reaction = [&]() -> void {
        current_set->emplace_back(reaction(sit, end, builder));
      };
      auto arrow_switch = [&]() -> void {
        if (!m_reactants.size())
          throw reactions::exceptions::__syntax_error("Missing reactants",
                                                      end - sit);

        if (current_set == &m_products)
          throw reactions::exceptions::__syntax_error("Duplicated arrow",
                                                      end - sit);

        current_set = &m_products;
      };

      processes::process_expression(sit, end, fill_element, fill_reaction,
                                    arrow_switch);

      if (!m_reactants.size())
        throw reactions::exceptions::__syntax_error("Missing reactants",
                                                    end - sit);

      if (!m_products.size())
        throw reactions::exceptions::__syntax_error("Missing products",
                                                    end - sit);
    }

    /// Reactants
    nodes_type m_reactants;
    /// Products
    nodes_type m_products;
  };

  /*! \brief Description of a process where head particle generate a set of
   * products
   *
   * This can be seen as a special reaction with only one reactant, and where
   * subsequent chain nodes are also decays.
   */
  template <class Element> class decay final {

  public:
    using element_type = element_wrapper<Element>;
    using builder_type = element_traits::builder_tpl_t<Element>;
    using nodes_type = std::vector<node<Element, decay>>;

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
             processes::detail::check_nodes(m_products, other.m_products);
    }

    /// Comparison operator
    bool operator!=(decay<Element> const &other) const {
      return !(*this == other);
    }

  protected:
    /*!\brief Constructor from the string iterators
     *
     * This constructor is called internally when creating decays that
     * contain other decays.
     *
     * \param begin iterator pointing to the beginning of the string.
     * \param end iterator pointing to the end of the string.
     * \param builder function to create the underlying element from
     * a string.
     */
    decay(std::string::const_iterator &&begin,
          std::string::const_iterator const &end, builder_type builder)
        : decay{begin, end, builder} {}

    template <class Process>
    friend Process
    reactions::processes::make_process(std::string const &,
                                       typename Process::builder_type);

    /*!\brief Constructor from the string iterators
     *
     * This constructor is called internally when creating decays that
     * contain other decays.
     *
     * \param sit iterator pointing to the current point int the string.
     * \param end iterator pointing to the end of the string.
     * \param builder function to create the underlying element from
     * a string.
     */
    decay(std::string::const_iterator &sit,
          std::string::const_iterator const &end, builder_type builder) {

      bool fill_products = false; // keep track of the elements we are adding

      auto fill_element =
          [&](std::string::const_iterator const &start) -> void {
        if (!m_head) {
          this->m_head.emplace(builder(std::string{start, sit}));
        } else if (fill_products) {
          this->m_products.emplace_back(builder(std::string{start, sit}));
        } else
          throw reactions::exceptions::__syntax_error("Missing arrow",
                                                      end - start);
      };
      auto fill_decay = [&] {
        if (!m_head) {
          throw reactions::exceptions::__syntax_error("Missing head",
                                                      end - sit);
        } else if (fill_products) {
          this->m_products.emplace_back(decay(sit, end, builder));
        } else
          throw reactions::exceptions::__syntax_error("Missing arrow",
                                                      end - sit);
      };
      auto arrow_switch = [&] {
        if (fill_products)
          throw reactions::exceptions::__syntax_error("Duplicated arrow",
                                                      end - sit);

        else if (!m_head)
          throw reactions::exceptions::__syntax_error("Missing head particle",
                                                      end - sit);

        fill_products = true;
      };

      processes::process_expression(sit, end, fill_element, fill_decay,
                                    arrow_switch);

      if (!m_head)
        throw reactions::exceptions::__syntax_error(
            "No elements have been parsed", end - sit);

      if (!m_products.size())
        throw reactions::exceptions::__syntax_error("Expected products",
                                                    end - sit);
    }

    /// Head particle
    std::optional<element_type> m_head;
    /// Products
    nodes_type m_products;
  };

  /*! \brief Create a new reaction with a custom builder
   *
   * \param str string to parse
   * \param builder function to build the underlying elements
   * \return the new reaction
   */
  template <class Element>
  reaction<Element>
  make_reaction_for(std::string const &str,
                    typename reaction<Element>::builder_type builder) {
    return processes::make_process<reaction<Element>>(str, builder);
  }

  /*! \brief Create a new reaction
   *
   * \param str string to process
   * \return the reaction
   */
  template <class Element>
  reaction<Element> make_reaction(std::string const &str) {
    return make_reaction_for<Element>(str, element_traits::builder<Element>);
  }

  /*! \brief Create a new decay with a custom builder
   *
   * \param str string to parse
   * \param builder function to build the underlying elements
   * \return the new decay
   */
  template <class Element>
  decay<Element> make_decay_for(std::string const &str,
                                typename decay<Element>::builder_type builder) {
    return processes::make_process<decay<Element>>(str, builder);
  }

  /*! \brief Create a new decay
   *
   * \param str string to process
   * \return the decay
   */
  template <class Element> decay<Element> make_decay(std::string const &str) {
    return make_decay_for<Element>(str, element_traits::builder<Element>);
  }
} // namespace reactions

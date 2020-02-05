#pragma once

#if __cplusplus >= 202000L
/// A keyword for defining a concept
#define CONCEPT concept
/**
 * Used when you want to type a template Concept, but keep it working to that
 * For instance:
 *
 * template<CONCEPT_TYPE(NumberConcept) TNumber>
 * void PrintNumber(TNumber number)
 * {
 * 	std::cout << number << '\n';
 * }
 */
#define CONCEPT_ARG(concept_name) concept_name

#define REQUIRES(...) [](__VA_ARGS__)
#define CONCEPT_FUNCTION(return_type, expression) { expression } -> return_type

#else
/// A keyword for defining a concept
#define CONCEPT constexpr bool
/**
 * Used when you want to type a template Concept, but keep it working to that
 * For instance:
 * 
 * template<CONCEPT_TYPE(NumberConcept) TNumber>
 * void PrintNumber(TNumber number)
 * {
 * 	std::cout << number << '\n';
 * }
 */
#define CONCEPT_TYPE(concept_name) typename 

#define REQUIRES(...) [](__VA_ARGS__)
// #define CONCEPT_FUNCTION(return_type, expression) static_assert(std::is_same_v<return_type, decltype(expression)>, "Function invocation '"#expression"' did not return '"#return_type"'")
#define CONCEPT_FUNCTION(return_type, expression) if(!std::is_same_v<return_type, decltype(expression)>) return false

#endif

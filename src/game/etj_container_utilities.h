/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <algorithm>

namespace ETJump {
namespace Container {
/**
 * Takes an input container (e.g. std::vector<>) and loops through each
 * item mapping it using the provided function. Returns a list of new items
 */
template <typename InputContainer, typename UnaryFunction>
auto map(const InputContainer &container, UnaryFunction &&func) {
  using InputType = typename InputContainer::value_type;
  using ResultType = std::invoke_result_t<UnaryFunction, InputType>;

  std::vector<ResultType> result;
  result.reserve(container.size());

  std::transform(container.begin(), container.end(), std::back_inserter(result),
                 std::forward<UnaryFunction>(func));

  return result;
}

/**
 * Takes an input container (e.g. std::vector<>) and loops through each item
 * filtering out ones that don't match the provided predicate. Returns a list
 * of filtered items
 */
template <typename InputContainer, typename Predicate>
auto filter(const InputContainer &container, Predicate &&pred) {
  using InputType = typename InputContainer::value_type;
  std::vector<InputType> result;

  std::copy_if(container.begin(), container.end(), std::back_inserter(result),
               std::forward<Predicate>(pred));

  return result;
}

/**
 * Checks if item is in container
 */
template <typename Container, typename Element>
bool isIn(const Container &container, const Element &element) {
  return std::find(container.begin(), container.end(), element) !=
         container.end();
}

template <typename TElement>
std::vector<TElement> skipFirstN(const std::vector<TElement> &input,
                                 int count) {
  if (count >= static_cast<int>(input.size())) {
    return std::vector<TElement>(); // Return an empty vector if count is
                                    // greater than or equal to the input size.
  }

  return std::vector<TElement>(input.begin() + count, input.end());
}
} // namespace Container
} // namespace ETJump

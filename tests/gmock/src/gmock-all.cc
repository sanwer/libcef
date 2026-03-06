// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//
// Google C++ Mocking Framework (Google Mock)
//
// This file #includes all Google Mock implementation .cc files.  The
// purpose is to allow a user to build Google Mock by compiling this
// file alone.

// This line ensures that gmock.h can be compiled on its own, even
// when it's fused.
#include "gmock/gmock.h"

// The following lines pull in the real gmock *.cc files.
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements cardinalities.


#include <limits.h>

#include <ostream>  // NOLINT
#include <sstream>
#include <string>


namespace testing {

namespace {

// Implements the Between(m, n) cardinality.
class BetweenCardinalityImpl : public CardinalityInterface {
 public:
  BetweenCardinalityImpl(int min, int max)
      : min_(min >= 0 ? min : 0), max_(max >= min_ ? max : min_) {
    std::stringstream ss;
    if (min < 0) {
      ss << "The invocation lower bound must be >= 0, " << "but is actually "
         << min << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (max < 0) {
      ss << "The invocation upper bound must be >= 0, " << "but is actually "
         << max << ".";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    } else if (min > max) {
      ss << "The invocation upper bound (" << max
         << ") must be >= the invocation lower bound (" << min << ").";
      internal::Expect(false, __FILE__, __LINE__, ss.str());
    }
  }

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  int ConservativeLowerBound() const override { return min_; }
  int ConservativeUpperBound() const override { return max_; }

  bool IsSatisfiedByCallCount(int call_count) const override {
    return min_ <= call_count && call_count <= max_;
  }

  bool IsSaturatedByCallCount(int call_count) const override {
    return call_count >= max_;
  }

  void DescribeTo(::std::ostream* os) const override;

 private:
  const int min_;
  const int max_;

  BetweenCardinalityImpl(const BetweenCardinalityImpl&) = delete;
  BetweenCardinalityImpl& operator=(const BetweenCardinalityImpl&) = delete;
};

// Formats "n times" in a human-friendly way.
inline std::string FormatTimes(int n) {
  if (n == 1) {
    return "once";
  } else if (n == 2) {
    return "twice";
  } else {
    std::stringstream ss;
    ss << n << " times";
    return ss.str();
  }
}

// Describes the Between(m, n) cardinality in human-friendly text.
void BetweenCardinalityImpl::DescribeTo(::std::ostream* os) const {
  if (min_ == 0) {
    if (max_ == 0) {
      *os << "never called";
    } else if (max_ == INT_MAX) {
      *os << "called any number of times";
    } else {
      *os << "called at most " << FormatTimes(max_);
    }
  } else if (min_ == max_) {
    *os << "called " << FormatTimes(min_);
  } else if (max_ == INT_MAX) {
    *os << "called at least " << FormatTimes(min_);
  } else {
    // 0 < min_ < max_ < INT_MAX
    *os << "called between " << min_ << " and " << max_ << " times";
  }
}

}  // Unnamed namespace

// Describes the given call count to an ostream.
void Cardinality::DescribeActualCallCountTo(int actual_call_count,
                                            ::std::ostream* os) {
  if (actual_call_count > 0) {
    *os << "called " << FormatTimes(actual_call_count);
  } else {
    *os << "never called";
  }
}

// Creates a cardinality that allows at least n calls.
GTEST_API_ Cardinality AtLeast(int n) { return Between(n, INT_MAX); }

// Creates a cardinality that allows at most n calls.
GTEST_API_ Cardinality AtMost(int n) { return Between(0, n); }

// Creates a cardinality that allows any number of calls.
GTEST_API_ Cardinality AnyNumber() { return AtLeast(0); }

// Creates a cardinality that allows between min and max calls.
GTEST_API_ Cardinality Between(int min, int max) {
  return Cardinality(new BetweenCardinalityImpl(min, max));
}

// Creates a cardinality that allows exactly n calls.
GTEST_API_ Cardinality Exactly(int n) { return Between(n, n); }

}  // namespace testing
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// This file defines some utilities useful for implementing Google
// Mock.  They are subject to change without notice, so please DO NOT
// USE THEM IN USER CODE.


#include <ctype.h>

#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>  // NOLINT
#include <string>
#include <utility>
#include <vector>


namespace testing {
namespace internal {

// Joins a vector of strings as if they are fields of a tuple; returns
// the joined string.
GTEST_API_ std::string JoinAsKeyValueTuple(
    const std::vector<const char*>& names, const Strings& values) {
  GTEST_CHECK_(names.size() == values.size());
  if (values.empty()) {
    return "";
  }
  const auto build_one = [&](const size_t i) {
    return std::string(names[i]) + ": " + values[i];
  };
  std::string result = "(" + build_one(0);
  for (size_t i = 1; i < values.size(); i++) {
    result += ", ";
    result += build_one(i);
  }
  result += ")";
  return result;
}

// Converts an identifier name to a space-separated list of lower-case
// words.  Each maximum substring of the form [A-Za-z][a-z]*|\d+ is
// treated as one word.  For example, both "FooBar123" and
// "foo_bar_123" are converted to "foo bar 123".
GTEST_API_ std::string ConvertIdentifierNameToWords(const char* id_name) {
  std::string result;
  char prev_char = '\0';
  for (const char* p = id_name; *p != '\0'; prev_char = *(p++)) {
    // We don't care about the current locale as the input is
    // guaranteed to be a valid C++ identifier name.
    const bool starts_new_word = IsUpper(*p) ||
                                 (!IsAlpha(prev_char) && IsLower(*p)) ||
                                 (!IsDigit(prev_char) && IsDigit(*p));

    if (IsAlNum(*p)) {
      if (starts_new_word && !result.empty()) result += ' ';
      result += ToLower(*p);
    }
  }
  return result;
}

// This class reports Google Mock failures as Google Test failures.  A
// user can define another class in a similar fashion if they intend to
// use Google Mock with a testing framework other than Google Test.
class GoogleTestFailureReporter : public FailureReporterInterface {
 public:
  void ReportFailure(FailureType type, const char* file, int line,
                     const std::string& message) override {
    AssertHelper(type == kFatal ? TestPartResult::kFatalFailure
                                : TestPartResult::kNonFatalFailure,
                 file, line, message.c_str()) = Message();
    if (type == kFatal) {
      posix::Abort();
    }
  }
};

// Returns the global failure reporter.  Will create a
// GoogleTestFailureReporter and return it the first time called.
GTEST_API_ FailureReporterInterface* GetFailureReporter() {
  // Points to the global failure reporter used by Google Mock.  gcc
  // guarantees that the following use of failure_reporter is
  // thread-safe.  We may need to add additional synchronization to
  // protect failure_reporter if we port Google Mock to other
  // compilers.
  static FailureReporterInterface* const failure_reporter =
      new GoogleTestFailureReporter();
  return failure_reporter;
}

// Protects global resources (stdout in particular) used by Log().
static GTEST_DEFINE_STATIC_MUTEX_(g_log_mutex);

// Returns true if and only if a log with the given severity is visible
// according to the --gmock_verbose flag.
GTEST_API_ bool LogIsVisible(LogSeverity severity) {
  if (GMOCK_FLAG_GET(verbose) == kInfoVerbosity) {
    // Always show the log if --gmock_verbose=info.
    return true;
  } else if (GMOCK_FLAG_GET(verbose) == kErrorVerbosity) {
    // Always hide it if --gmock_verbose=error.
    return false;
  } else {
    // If --gmock_verbose is neither "info" nor "error", we treat it
    // as "warning" (its default value).
    return severity == kWarning;
  }
}

// Prints the given message to stdout if and only if 'severity' >= the level
// specified by the --gmock_verbose flag.  If stack_frames_to_skip >=
// 0, also prints the stack trace excluding the top
// stack_frames_to_skip frames.  In opt mode, any positive
// stack_frames_to_skip is treated as 0, since we don't know which
// function calls will be inlined by the compiler and need to be
// conservative.
GTEST_API_ void Log(LogSeverity severity, const std::string& message,
                    int stack_frames_to_skip) {
  if (!LogIsVisible(severity)) return;

  // Ensures that logs from different threads don't interleave.
  MutexLock l(&g_log_mutex);

  if (severity == kWarning) {
    // Prints a GMOCK WARNING marker to make the warnings easily searchable.
    std::cout << "\nGMOCK WARNING:";
  }
  // Pre-pends a new-line to message if it doesn't start with one.
  if (message.empty() || message[0] != '\n') {
    std::cout << "\n";
  }
  std::cout << message;
  if (stack_frames_to_skip >= 0) {
#ifdef NDEBUG
    // In opt mode, we have to be conservative and skip no stack frame.
    const int actual_to_skip = 0;
#else
    // In dbg mode, we can do what the caller tell us to do (plus one
    // for skipping this function's stack frame).
    const int actual_to_skip = stack_frames_to_skip + 1;
#endif  // NDEBUG

    // Appends a new-line to message if it doesn't end with one.
    if (!message.empty() && *message.rbegin() != '\n') {
      std::cout << "\n";
    }
    std::cout << "Stack trace:\n"
              << ::testing::internal::GetCurrentOsStackTraceExceptTop(
                     actual_to_skip);
  }
  std::cout << ::std::flush;
}

GTEST_API_ WithoutMatchers GetWithoutMatchers() { return WithoutMatchers(); }

GTEST_API_ void IllegalDoDefault(const char* file, int line) {
  internal::Assert(
      false, file, line,
      "You are using DoDefault() inside a composite action like "
      "DoAll() or WithArgs().  This is not supported for technical "
      "reasons.  Please instead spell out the default action, or "
      "assign the default action to an Action variable and use "
      "the variable in various places.");
}

constexpr char UndoWebSafeEncoding(char c) {
  return c == '-' ? '+' : c == '_' ? '/' : c;
}

constexpr char UnBase64Impl(char c, const char* const base64, char carry) {
  return *base64 == 0 ? static_cast<char>(65)
         : *base64 == c
             ? carry
             : UnBase64Impl(c, base64 + 1, static_cast<char>(carry + 1));
}

template <size_t... I>
constexpr std::array<char, 256> UnBase64Impl(std::index_sequence<I...>,
                                             const char* const base64) {
  return {
      {UnBase64Impl(UndoWebSafeEncoding(static_cast<char>(I)), base64, 0)...}};
}

constexpr std::array<char, 256> UnBase64(const char* const base64) {
  return UnBase64Impl(std::make_index_sequence<256>{}, base64);
}

static constexpr char kBase64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static constexpr std::array<char, 256> kUnBase64 = UnBase64(kBase64);

bool Base64Unescape(const std::string& encoded, std::string* decoded) {
  decoded->clear();
  size_t encoded_len = encoded.size();
  decoded->reserve(3 * (encoded_len / 4) + (encoded_len % 4));
  int bit_pos = 0;
  char dst = 0;
  for (int src : encoded) {
    if (std::isspace(src) || src == '=') {
      continue;
    }
    char src_bin = kUnBase64[static_cast<size_t>(src)];
    if (src_bin >= 64) {
      decoded->clear();
      return false;
    }
    if (bit_pos == 0) {
      dst |= static_cast<char>(src_bin << 2);
      bit_pos = 6;
    } else {
      dst |= static_cast<char>(src_bin >> (bit_pos - 2));
      decoded->push_back(dst);
      dst = static_cast<char>(src_bin << (10 - bit_pos));
      bit_pos = (bit_pos + 6) % 8;
    }
  }
  return true;
}

}  // namespace internal
}  // namespace testing
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements Matcher<const string&>, Matcher<string>, and
// utilities for defining matchers.


#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testing {
namespace internal {

// Returns the description for a matcher defined using the MATCHER*()
// macro where the user-supplied description string is "", if
// 'negation' is false; otherwise returns the description of the
// negation of the matcher.  'param_values' contains a list of strings
// that are the print-out of the matcher's parameters.
GTEST_API_ std::string FormatMatcherDescription(
    bool negation, const char* matcher_name,
    const std::vector<const char*>& param_names, const Strings& param_values) {
  std::string result = ConvertIdentifierNameToWords(matcher_name);
  if (!param_values.empty()) {
    result += " " + JoinAsKeyValueTuple(param_names, param_values);
  }
  return negation ? "not (" + result + ")" : result;
}

// FindMaxBipartiteMatching and its helper class.
//
// Uses the well-known Ford-Fulkerson max flow method to find a maximum
// bipartite matching. Flow is considered to be from left to right.
// There is an implicit source node that is connected to all of the left
// nodes, and an implicit sink node that is connected to all of the
// right nodes. All edges have unit capacity.
//
// Neither the flow graph nor the residual flow graph are represented
// explicitly. Instead, they are implied by the information in 'graph' and
// a vector<int> called 'left_' whose elements are initialized to the
// value kUnused. This represents the initial state of the algorithm,
// where the flow graph is empty, and the residual flow graph has the
// following edges:
//   - An edge from source to each left_ node
//   - An edge from each right_ node to sink
//   - An edge from each left_ node to each right_ node, if the
//     corresponding edge exists in 'graph'.
//
// When the TryAugment() method adds a flow, it sets left_[l] = r for some
// nodes l and r. This induces the following changes:
//   - The edges (source, l), (l, r), and (r, sink) are added to the
//     flow graph.
//   - The same three edges are removed from the residual flow graph.
//   - The reverse edges (l, source), (r, l), and (sink, r) are added
//     to the residual flow graph, which is a directional graph
//     representing unused flow capacity.
//
// When the method augments a flow (moving left_[l] from some r1 to some
// other r2), this can be thought of as "undoing" the above steps with
// respect to r1 and "redoing" them with respect to r2.
//
// It bears repeating that the flow graph and residual flow graph are
// never represented explicitly, but can be derived by looking at the
// information in 'graph' and in left_.
//
// As an optimization, there is a second vector<int> called right_ which
// does not provide any new information. Instead, it enables more
// efficient queries about edges entering or leaving the right-side nodes
// of the flow or residual flow graphs. The following invariants are
// maintained:
//
// left[l] == kUnused or right[left[l]] == l
// right[r] == kUnused or left[right[r]] == r
//
// . [ source ]                                        .
// .   |||                                             .
// .   |||                                             .
// .   ||\--> left[0]=1  ---\    right[0]=-1 ----\     .
// .   ||                   |                    |     .
// .   |\---> left[1]=-1    \--> right[1]=0  ---\|     .
// .   |                                        ||     .
// .   \----> left[2]=2  ------> right[2]=2  --\||     .
// .                                           |||     .
// .         elements           matchers       vvv     .
// .                                         [ sink ]  .
//
// See Also:
//   [1] Cormen, et al (2001). "Section 26.2: The Ford-Fulkerson method".
//       "Introduction to Algorithms (Second ed.)", pp. 651-664.
//   [2] "Ford-Fulkerson algorithm", Wikipedia,
//       'https://en.wikipedia.org/wiki/Ford%E2%80%93Fulkerson_algorithm'
class MaxBipartiteMatchState {
 public:
  explicit MaxBipartiteMatchState(const MatchMatrix& graph)
      : graph_(&graph),
        left_(graph_->LhsSize(), kUnused),
        right_(graph_->RhsSize(), kUnused) {}

  // Returns the edges of a maximal match, each in the form {left, right}.
  ElementMatcherPairs Compute() {
    // 'seen' is used for path finding { 0: unseen, 1: seen }.
    ::std::vector<char> seen;
    // Searches the residual flow graph for a path from each left node to
    // the sink in the residual flow graph, and if one is found, add flow
    // to the graph. It's okay to search through the left nodes once. The
    // edge from the implicit source node to each previously-visited left
    // node will have flow if that left node has any path to the sink
    // whatsoever. Subsequent augmentations can only add flow to the
    // network, and cannot take away that previous flow unit from the source.
    // Since the source-to-left edge can only carry one flow unit (or,
    // each element can be matched to only one matcher), there is no need
    // to visit the left nodes more than once looking for augmented paths.
    // The flow is known to be possible or impossible by looking at the
    // node once.
    for (size_t ilhs = 0; ilhs < graph_->LhsSize(); ++ilhs) {
      // Reset the path-marking vector and try to find a path from
      // source to sink starting at the left_[ilhs] node.
      GTEST_CHECK_(left_[ilhs] == kUnused)
          << "ilhs: " << ilhs << ", left_[ilhs]: " << left_[ilhs];
      // 'seen' initialized to 'graph_->RhsSize()' copies of 0.
      seen.assign(graph_->RhsSize(), 0);
      TryAugment(ilhs, &seen);
    }
    ElementMatcherPairs result;
    for (size_t ilhs = 0; ilhs < left_.size(); ++ilhs) {
      size_t irhs = left_[ilhs];
      if (irhs == kUnused) continue;
      result.push_back(ElementMatcherPair(ilhs, irhs));
    }
    return result;
  }

 private:
  static const size_t kUnused = static_cast<size_t>(-1);

  // Perform a depth-first search from left node ilhs to the sink.  If a
  // path is found, flow is added to the network by linking the left and
  // right vector elements corresponding each segment of the path.
  // Returns true if a path to sink was found, which means that a unit of
  // flow was added to the network. The 'seen' vector elements correspond
  // to right nodes and are marked to eliminate cycles from the search.
  //
  // Left nodes will only be explored at most once because they
  // are accessible from at most one right node in the residual flow
  // graph.
  //
  // Note that left_[ilhs] is the only element of left_ that TryAugment will
  // potentially transition from kUnused to another value. Any other
  // left_ element holding kUnused before TryAugment will be holding it
  // when TryAugment returns.
  //
  bool TryAugment(size_t ilhs, ::std::vector<char>* seen) {
    for (size_t irhs = 0; irhs < graph_->RhsSize(); ++irhs) {
      if ((*seen)[irhs]) continue;
      if (!graph_->HasEdge(ilhs, irhs)) continue;
      // There's an available edge from ilhs to irhs.
      (*seen)[irhs] = 1;
      // Next a search is performed to determine whether
      // this edge is a dead end or leads to the sink.
      //
      // right_[irhs] == kUnused means that there is residual flow from
      // right node irhs to the sink, so we can use that to finish this
      // flow path and return success.
      //
      // Otherwise there is residual flow to some ilhs. We push flow
      // along that path and call ourselves recursively to see if this
      // ultimately leads to sink.
      if (right_[irhs] == kUnused || TryAugment(right_[irhs], seen)) {
        // Add flow from left_[ilhs] to right_[irhs].
        left_[ilhs] = irhs;
        right_[irhs] = ilhs;
        return true;
      }
    }
    return false;
  }

  const MatchMatrix* graph_;  // not owned
  // Each element of the left_ vector represents a left hand side node
  // (i.e. an element) and each element of right_ is a right hand side
  // node (i.e. a matcher). The values in the left_ vector indicate
  // outflow from that node to a node on the right_ side. The values
  // in the right_ indicate inflow, and specify which left_ node is
  // feeding that right_ node, if any. For example, left_[3] == 1 means
  // there's a flow from element #3 to matcher #1. Such a flow would also
  // be redundantly represented in the right_ vector as right_[1] == 3.
  // Elements of left_ and right_ are either kUnused or mutually
  // referent. Mutually referent means that left_[right_[i]] = i and
  // right_[left_[i]] = i.
  ::std::vector<size_t> left_;
  ::std::vector<size_t> right_;
};

const size_t MaxBipartiteMatchState::kUnused;

GTEST_API_ ElementMatcherPairs FindMaxBipartiteMatching(const MatchMatrix& g) {
  return MaxBipartiteMatchState(g).Compute();
}

static void LogElementMatcherPairVec(const ElementMatcherPairs& pairs,
                                     ::std::ostream* stream) {
  typedef ElementMatcherPairs::const_iterator Iter;
  ::std::ostream& os = *stream;
  os << "{";
  const char* sep = "";
  for (Iter it = pairs.begin(); it != pairs.end(); ++it) {
    os << sep << "\n  (" << "element #" << it->first << ", " << "matcher #"
       << it->second << ")";
    sep = ",";
  }
  os << "\n}";
}

bool MatchMatrix::NextGraph() {
  for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
    for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
      char& b = matched_[SpaceIndex(ilhs, irhs)];
      if (!b) {
        b = 1;
        return true;
      }
      b = 0;
    }
  }
  return false;
}

void MatchMatrix::Randomize() {
  for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
    for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
      char& b = matched_[SpaceIndex(ilhs, irhs)];
      b = static_cast<char>(rand() & 1);  // NOLINT
    }
  }
}

std::string MatchMatrix::DebugString() const {
  ::std::stringstream ss;
  const char* sep = "";
  for (size_t i = 0; i < LhsSize(); ++i) {
    ss << sep;
    for (size_t j = 0; j < RhsSize(); ++j) {
      ss << HasEdge(i, j);
    }
    sep = ";";
  }
  return ss.str();
}

void UnorderedElementsAreMatcherImplBase::DescribeToImpl(
    ::std::ostream* os) const {
  switch (match_flags()) {
    case UnorderedMatcherRequire::ExactMatch:
      if (matcher_describers_.empty()) {
        *os << "is empty";
        return;
      }
      if (matcher_describers_.size() == 1) {
        *os << "has " << Elements(1) << " and that element ";
        matcher_describers_[0]->DescribeTo(os);
        return;
      }
      *os << "has " << Elements(matcher_describers_.size())
          << " and there exists some permutation of elements such that:\n";
      break;
    case UnorderedMatcherRequire::Superset:
      *os << "a surjection from elements to requirements exists such that:\n";
      break;
    case UnorderedMatcherRequire::Subset:
      *os << "an injection from elements to requirements exists such that:\n";
      break;
  }

  const char* sep = "";
  for (size_t i = 0; i != matcher_describers_.size(); ++i) {
    *os << sep;
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      *os << " - element #" << i << " ";
    } else {
      *os << " - an element ";
    }
    matcher_describers_[i]->DescribeTo(os);
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      sep = ", and\n";
    } else {
      sep = "\n";
    }
  }
}

void UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(
    ::std::ostream* os) const {
  switch (match_flags()) {
    case UnorderedMatcherRequire::ExactMatch:
      if (matcher_describers_.empty()) {
        *os << "isn't empty";
        return;
      }
      if (matcher_describers_.size() == 1) {
        *os << "doesn't have " << Elements(1) << ", or has " << Elements(1)
            << " that ";
        matcher_describers_[0]->DescribeNegationTo(os);
        return;
      }
      *os << "doesn't have " << Elements(matcher_describers_.size())
          << ", or there exists no permutation of elements such that:\n";
      break;
    case UnorderedMatcherRequire::Superset:
      *os << "no surjection from elements to requirements exists such that:\n";
      break;
    case UnorderedMatcherRequire::Subset:
      *os << "no injection from elements to requirements exists such that:\n";
      break;
  }
  const char* sep = "";
  for (size_t i = 0; i != matcher_describers_.size(); ++i) {
    *os << sep;
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      *os << " - element #" << i << " ";
    } else {
      *os << " - an element ";
    }
    matcher_describers_[i]->DescribeTo(os);
    if (match_flags() == UnorderedMatcherRequire::ExactMatch) {
      sep = ", and\n";
    } else {
      sep = "\n";
    }
  }
}

// Checks that all matchers match at least one element, and that all
// elements match at least one matcher. This enables faster matching
// and better error reporting.
// Returns false, writing an explanation to 'listener', if and only
// if the success criteria are not met.
bool UnorderedElementsAreMatcherImplBase::VerifyMatchMatrix(
    const ::std::vector<std::string>& element_printouts,
    const MatchMatrix& matrix, MatchResultListener* listener) const {
  if (matrix.LhsSize() == 0 && matrix.RhsSize() == 0) {
    return true;
  }

  const bool is_exact_match_with_size_discrepency =
      match_flags() == UnorderedMatcherRequire::ExactMatch &&
      matrix.LhsSize() != matrix.RhsSize();
  if (is_exact_match_with_size_discrepency) {
    // The element count doesn't match.  If the container is empty,
    // there's no need to explain anything as Google Mock already
    // prints the empty container. Otherwise we just need to show
    // how many elements there actually are.
    if (matrix.LhsSize() != 0 && listener->IsInterested()) {
      *listener << "which has " << Elements(matrix.LhsSize()) << "\n";
    }
  }

  bool result = !is_exact_match_with_size_discrepency;
  ::std::vector<char> element_matched(matrix.LhsSize(), 0);
  ::std::vector<char> matcher_matched(matrix.RhsSize(), 0);

  for (size_t ilhs = 0; ilhs < matrix.LhsSize(); ilhs++) {
    for (size_t irhs = 0; irhs < matrix.RhsSize(); irhs++) {
      char matched = matrix.HasEdge(ilhs, irhs);
      element_matched[ilhs] |= matched;
      matcher_matched[irhs] |= matched;
    }
  }

  if (match_flags() & UnorderedMatcherRequire::Superset) {
    const char* sep =
        "where the following matchers don't match any elements:\n";
    for (size_t mi = 0; mi < matcher_matched.size(); ++mi) {
      if (matcher_matched[mi]) continue;
      result = false;
      if (listener->IsInterested()) {
        *listener << sep << "matcher #" << mi << ": ";
        matcher_describers_[mi]->DescribeTo(listener->stream());
        sep = ",\n";
      }
    }
  }

  if (match_flags() & UnorderedMatcherRequire::Subset) {
    const char* sep =
        "where the following elements don't match any matchers:\n";
    const char* outer_sep = "";
    if (!result) {
      outer_sep = "\nand ";
    }
    for (size_t ei = 0; ei < element_matched.size(); ++ei) {
      if (element_matched[ei]) continue;
      result = false;
      if (listener->IsInterested()) {
        *listener << outer_sep << sep << "element #" << ei << ": "
                  << element_printouts[ei];
        sep = ",\n";
        outer_sep = "";
      }
    }
  }
  return result;
}

bool UnorderedElementsAreMatcherImplBase::FindPairing(
    const MatchMatrix& matrix, MatchResultListener* listener) const {
  ElementMatcherPairs matches = FindMaxBipartiteMatching(matrix);

  size_t max_flow = matches.size();
  if ((match_flags() & UnorderedMatcherRequire::Superset) &&
      max_flow < matrix.RhsSize()) {
    if (listener->IsInterested()) {
      *listener << "where no permutation of the elements can satisfy all "
                   "matchers, and the closest match is "
                << max_flow << " of " << matrix.RhsSize()
                << " matchers with the pairings:\n";
      LogElementMatcherPairVec(matches, listener->stream());
    }
    return false;
  }
  if ((match_flags() & UnorderedMatcherRequire::Subset) &&
      max_flow < matrix.LhsSize()) {
    if (listener->IsInterested()) {
      *listener
          << "where not all elements can be matched, and the closest match is "
          << max_flow << " of " << matrix.RhsSize()
          << " matchers with the pairings:\n";
      LogElementMatcherPairVec(matches, listener->stream());
    }
    return false;
  }

  if (matches.size() > 1) {
    if (listener->IsInterested()) {
      const char* sep = "where:\n";
      for (size_t mi = 0; mi < matches.size(); ++mi) {
        *listener << sep << " - element #" << matches[mi].first
                  << " is matched by matcher #" << matches[mi].second;
        sep = ",\n";
      }
    }
  }
  return true;
}

}  // namespace internal
}  // namespace testing
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements the spec builder syntax (ON_CALL and
// EXPECT_CALL).


#include <stdlib.h>

#include <iostream>  // NOLINT
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


#if defined(GTEST_OS_CYGWIN) || defined(GTEST_OS_LINUX) || defined(GTEST_OS_MAC)
#include <unistd.h>  // NOLINT
#endif
#ifdef GTEST_OS_QURT
#include <qurt_event.h>
#endif

// Silence C4800 (C4800: 'int *const ': forcing value
// to bool 'true' or 'false') for MSVC 15
#if defined(_MSC_VER) && (_MSC_VER == 1900)
GTEST_DISABLE_MSC_WARNINGS_PUSH_(4800)
#endif

namespace testing {
namespace internal {

// Protects the mock object registry (in class Mock), all function
// mockers, and all expectations.
GTEST_API_ GTEST_DEFINE_STATIC_MUTEX_(g_gmock_mutex);

// Logs a message including file and line number information.
GTEST_API_ void LogWithLocation(testing::internal::LogSeverity severity,
                                const char* file, int line,
                                const std::string& message) {
  ::std::ostringstream s;
  s << internal::FormatFileLocation(file, line) << " " << message
    << ::std::endl;
  Log(severity, s.str(), 0);
}

// Constructs an ExpectationBase object.
ExpectationBase::ExpectationBase(const char* a_file, int a_line,
                                 const std::string& a_source_text)
    : file_(a_file),
      line_(a_line),
      source_text_(a_source_text),
      cardinality_specified_(false),
      cardinality_(Exactly(1)),
      call_count_(0),
      retired_(false),
      extra_matcher_specified_(false),
      repeated_action_specified_(false),
      retires_on_saturation_(false),
      last_clause_(kNone),
      action_count_checked_(false) {}

// Destructs an ExpectationBase object.
ExpectationBase::~ExpectationBase() = default;

// Explicitly specifies the cardinality of this expectation.  Used by
// the subclasses to implement the .Times() clause.
void ExpectationBase::SpecifyCardinality(const Cardinality& a_cardinality) {
  cardinality_specified_ = true;
  cardinality_ = a_cardinality;
}

// Retires all pre-requisites of this expectation.
void ExpectationBase::RetireAllPreRequisites()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  if (is_retired()) {
    // We can take this short-cut as we never retire an expectation
    // until we have retired all its pre-requisites.
    return;
  }

  ::std::vector<ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      ExpectationBase* next = it->expectation_base().get();
      if (!next->is_retired()) {
        next->Retire();
        expectations.push_back(next);
      }
    }
  }
}

// Returns true if and only if all pre-requisites of this expectation
// have been satisfied.
bool ExpectationBase::AllPrerequisitesAreSatisfied() const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();
      if (!next->IsSatisfied()) return false;
      expectations.push_back(next);
    }
  }
  return true;
}

// Adds unsatisfied pre-requisites of this expectation to 'result'.
void ExpectationBase::FindUnsatisfiedPrerequisites(ExpectationSet* result) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  ::std::vector<const ExpectationBase*> expectations(1, this);
  while (!expectations.empty()) {
    const ExpectationBase* exp = expectations.back();
    expectations.pop_back();

    for (ExpectationSet::const_iterator it =
             exp->immediate_prerequisites_.begin();
         it != exp->immediate_prerequisites_.end(); ++it) {
      const ExpectationBase* next = it->expectation_base().get();

      if (next->IsSatisfied()) {
        // If *it is satisfied and has a call count of 0, some of its
        // pre-requisites may not be satisfied yet.
        if (next->call_count_ == 0) {
          expectations.push_back(next);
        }
      } else {
        // Now that we know next is unsatisfied, we are not so interested
        // in whether its pre-requisites are satisfied.  Therefore we
        // don't iterate into it here.
        *result += *it;
      }
    }
  }
}

// Describes how many times a function call matching this
// expectation has occurred.
void ExpectationBase::DescribeCallCountTo(::std::ostream* os) const
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();

  // Describes how many times the function is expected to be called.
  *os << "         Expected: to be ";
  cardinality().DescribeTo(os);
  *os << "\n           Actual: ";
  Cardinality::DescribeActualCallCountTo(call_count(), os);

  // Describes the state of the expectation (e.g. is it satisfied?
  // is it active?).
  *os << " - "
      << (IsOverSaturated() ? "over-saturated"
          : IsSaturated()   ? "saturated"
          : IsSatisfied()   ? "satisfied"
                            : "unsatisfied")
      << " and " << (is_retired() ? "retired" : "active");
}

// Checks the action count (i.e. the number of WillOnce() and
// WillRepeatedly() clauses) against the cardinality if this hasn't
// been done before.  Prints a warning if there are too many or too
// few actions.
void ExpectationBase::CheckActionCountIfNotDone() const
    GTEST_LOCK_EXCLUDED_(mutex_) {
  bool should_check = false;
  {
    MutexLock l(&mutex_);
    if (!action_count_checked_) {
      action_count_checked_ = true;
      should_check = true;
    }
  }

  if (should_check) {
    if (!cardinality_specified_) {
      // The cardinality was inferred - no need to check the action
      // count against it.
      return;
    }

    // The cardinality was explicitly specified.
    const int action_count = static_cast<int>(untyped_actions_.size());
    const int upper_bound = cardinality().ConservativeUpperBound();
    const int lower_bound = cardinality().ConservativeLowerBound();
    bool too_many;  // True if there are too many actions, or false
    // if there are too few.
    if (action_count > upper_bound ||
        (action_count == upper_bound && repeated_action_specified_)) {
      too_many = true;
    } else if (0 < action_count && action_count < lower_bound &&
               !repeated_action_specified_) {
      too_many = false;
    } else {
      return;
    }

    ::std::stringstream ss;
    DescribeLocationTo(&ss);
    ss << "Too " << (too_many ? "many" : "few") << " actions specified in "
       << source_text() << "...\n"
       << "Expected to be ";
    cardinality().DescribeTo(&ss);
    ss << ", but has " << (too_many ? "" : "only ") << action_count
       << " WillOnce()" << (action_count == 1 ? "" : "s");
    if (repeated_action_specified_) {
      ss << " and a WillRepeatedly()";
    }
    ss << ".";
    Log(kWarning, ss.str(), -1);  // -1 means "don't print stack trace".
  }
}

// Implements the .Times() clause.
void ExpectationBase::UntypedTimes(const Cardinality& a_cardinality) {
  if (last_clause_ == kTimes) {
    ExpectSpecProperty(false,
                       ".Times() cannot appear "
                       "more than once in an EXPECT_CALL().");
  } else {
    ExpectSpecProperty(
        last_clause_ < kTimes,
        ".Times() may only appear *before* .InSequence(), .WillOnce(), "
        ".WillRepeatedly(), or .RetiresOnSaturation(), not after.");
  }
  last_clause_ = kTimes;

  SpecifyCardinality(a_cardinality);
}

// Points to the implicit sequence introduced by a living InSequence
// object (if any) in the current thread or NULL.
GTEST_API_ ThreadLocal<Sequence*> g_gmock_implicit_sequence;

// Reports an uninteresting call (whose description is in msg) in the
// manner specified by 'reaction'.
void ReportUninterestingCall(CallReaction reaction, const std::string& msg) {
  // Include a stack trace only if --gmock_verbose=info is specified.
  const int stack_frames_to_skip =
      GMOCK_FLAG_GET(verbose) == kInfoVerbosity ? 3 : -1;
  switch (reaction) {
    case kAllow:
      Log(kInfo, msg, stack_frames_to_skip);
      break;
    case kWarn:
      Log(kWarning,
          msg +
              "\nNOTE: You can safely ignore the above warning unless this "
              "call should not happen.  Do not suppress it by adding "
              "an EXPECT_CALL() if you don't mean to enforce the call.  "
              "See "
              "https://github.com/google/googletest/blob/main/docs/"
              "gmock_cook_book.md#"
              "knowing-when-to-expect-useoncall for details.\n",
          stack_frames_to_skip);
      break;
    default:  // FAIL
      Expect(false, nullptr, -1, msg);
  }
}

UntypedFunctionMockerBase::UntypedFunctionMockerBase()
    : mock_obj_(nullptr), name_("") {}

UntypedFunctionMockerBase::~UntypedFunctionMockerBase() = default;

// Sets the mock object this mock method belongs to, and registers
// this information in the global mock registry.  Will be called
// whenever an EXPECT_CALL() or ON_CALL() is executed on this mock
// method.
void UntypedFunctionMockerBase::RegisterOwner(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  {
    MutexLock l(&g_gmock_mutex);
    mock_obj_ = mock_obj;
  }
  Mock::Register(mock_obj, this);
}

// Sets the mock object this mock method belongs to, and sets the name
// of the mock function.  Will be called upon each invocation of this
// mock function.
void UntypedFunctionMockerBase::SetOwnerAndName(const void* mock_obj,
                                                const char* name)
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  // We protect name_ under g_gmock_mutex in case this mock function
  // is called from two threads concurrently.
  MutexLock l(&g_gmock_mutex);
  mock_obj_ = mock_obj;
  name_ = name;
}

// Returns the name of the function being mocked.  Must be called
// after RegisterOwner() or SetOwnerAndName() has been called.
const void* UntypedFunctionMockerBase::MockObject() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const void* mock_obj;
  {
    // We protect mock_obj_ under g_gmock_mutex in case this mock
    // function is called from two threads concurrently.
    MutexLock l(&g_gmock_mutex);
    Assert(mock_obj_ != nullptr, __FILE__, __LINE__,
           "MockObject() must not be called before RegisterOwner() or "
           "SetOwnerAndName() has been called.");
    mock_obj = mock_obj_;
  }
  return mock_obj;
}

// Returns the name of this mock method.  Must be called after
// SetOwnerAndName() has been called.
const char* UntypedFunctionMockerBase::Name() const
    GTEST_LOCK_EXCLUDED_(g_gmock_mutex) {
  const char* name;
  {
    // We protect name_ under g_gmock_mutex in case this mock
    // function is called from two threads concurrently.
    MutexLock l(&g_gmock_mutex);
    Assert(name_ != nullptr, __FILE__, __LINE__,
           "Name() must not be called before SetOwnerAndName() has "
           "been called.");
    name = name_;
  }
  return name;
}

// Returns an Expectation object that references and co-owns exp,
// which must be an expectation on this mock function.
Expectation UntypedFunctionMockerBase::GetHandleOf(ExpectationBase* exp) {
  // See the definition of untyped_expectations_ for why access to it
  // is unprotected here.
  for (UntypedExpectations::const_iterator it = untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    if (it->get() == exp) {
      return Expectation(*it);
    }
  }

  Assert(false, __FILE__, __LINE__, "Cannot find expectation.");
  return Expectation();
  // The above statement is just to make the code compile, and will
  // never be executed.
}

// Verifies that all expectations on this mock function have been
// satisfied.  Reports one or more Google Test non-fatal failures
// and returns false if not.
bool UntypedFunctionMockerBase::VerifyAndClearExpectationsLocked()
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(g_gmock_mutex) {
  g_gmock_mutex.AssertHeld();
  bool expectations_met = true;
  for (UntypedExpectations::const_iterator it = untyped_expectations_.begin();
       it != untyped_expectations_.end(); ++it) {
    ExpectationBase* const untyped_expectation = it->get();
    if (untyped_expectation->IsOverSaturated()) {
      // There was an upper-bound violation.  Since the error was
      // already reported when it occurred, there is no need to do
      // anything here.
      expectations_met = false;
    } else if (!untyped_expectation->IsSatisfied()) {
      expectations_met = false;
      ::std::stringstream ss;

      const ::std::string& expectation_name =
          untyped_expectation->GetDescription();
      ss << "Actual function ";
      if (!expectation_name.empty()) {
        ss << "\"" << expectation_name << "\" ";
      }
      ss << "call count doesn't match " << untyped_expectation->source_text()
         << "...\n";
      // No need to show the source file location of the expectation
      // in the description, as the Expect() call that follows already
      // takes care of it.
      untyped_expectation->MaybeDescribeExtraMatcherTo(&ss);
      untyped_expectation->DescribeCallCountTo(&ss);
      Expect(false, untyped_expectation->file(), untyped_expectation->line(),
             ss.str());
    }
  }

  // Deleting our expectations may trigger other mock objects to be deleted, for
  // example if an action contains a reference counted smart pointer to that
  // mock object, and that is the last reference. So if we delete our
  // expectations within the context of the global mutex we may deadlock when
  // this method is called again. Instead, make a copy of the set of
  // expectations to delete, clear our set within the mutex, and then clear the
  // copied set outside of it.
  UntypedExpectations expectations_to_delete;
  untyped_expectations_.swap(expectations_to_delete);

  g_gmock_mutex.unlock();
  expectations_to_delete.clear();
  g_gmock_mutex.lock();

  return expectations_met;
}

static CallReaction intToCallReaction(int mock_behavior) {
  if (mock_behavior >= kAllow && mock_behavior <= kFail) {
    return static_cast<internal::CallReaction>(mock_behavior);
  }
  return kWarn;
}

}  // namespace internal

// Class Mock.

namespace {

typedef std::set<internal::UntypedFunctionMockerBase*> FunctionMockers;

// The current state of a mock object.  Such information is needed for
// detecting leaked mock objects and explicitly verifying a mock's
// expectations.
struct MockObjectState {
  MockObjectState()
      : first_used_file(nullptr), first_used_line(-1), leakable(false) {}

  // Where in the source file an ON_CALL or EXPECT_CALL is first
  // invoked on this mock object.
  const char* first_used_file;
  int first_used_line;
  ::std::string first_used_test_suite;
  ::std::string first_used_test;
  bool leakable;  // true if and only if it's OK to leak the object.
  FunctionMockers function_mockers;  // All registered methods of the object.
};

// A global registry holding the state of all mock objects that are
// alive.  A mock object is added to this registry the first time
// Mock::AllowLeak(), ON_CALL(), or EXPECT_CALL() is called on it.  It
// is removed from the registry in the mock object's destructor.
class MockObjectRegistry {
 public:
  // Maps a mock object (identified by its address) to its state.
  typedef std::map<const void*, MockObjectState> StateMap;

  // This destructor will be called when a program exits, after all
  // tests in it have been run.  By then, there should be no mock
  // object alive.  Therefore we report any living object as test
  // failure, unless the user explicitly asked us to ignore it.
  ~MockObjectRegistry() {
    if (!GMOCK_FLAG_GET(catch_leaked_mocks)) return;
    internal::MutexLock l(&internal::g_gmock_mutex);

    int leaked_count = 0;
    for (StateMap::const_iterator it = states_.begin(); it != states_.end();
         ++it) {
      if (it->second.leakable)  // The user said it's fine to leak this object.
        continue;

      // FIXME: Print the type of the leaked object.
      // This can help the user identify the leaked object.
      std::cout << "\n";
      const MockObjectState& state = it->second;
      std::cout << internal::FormatFileLocation(state.first_used_file,
                                                state.first_used_line);
      std::cout << " ERROR: this mock object";
      if (!state.first_used_test.empty()) {
        std::cout << " (used in test " << state.first_used_test_suite << "."
                  << state.first_used_test << ")";
      }
      std::cout << " should be deleted but never is. Its address is @"
                << it->first << ".";
      leaked_count++;
    }
    if (leaked_count > 0) {
      std::cout << "\nERROR: " << leaked_count << " leaked mock "
                << (leaked_count == 1 ? "object" : "objects")
                << " found at program exit. Expectations on a mock object are "
                   "verified when the object is destructed. Leaking a mock "
                   "means that its expectations aren't verified, which is "
                   "usually a test bug. If you really intend to leak a mock, "
                   "you can suppress this error using "
                   "testing::Mock::AllowLeak(mock_object), or you may use a "
                   "fake or stub instead of a mock.\n";
      std::cout.flush();
      ::std::cerr.flush();
      // RUN_ALL_TESTS() has already returned when this destructor is
      // called.  Therefore we cannot use the normal Google Test
      // failure reporting mechanism.
#ifdef GTEST_OS_QURT
      qurt_exception_raise_fatal();
#else
      _Exit(1);  // We cannot call exit() as it is not reentrant and
                 // may already have been called.
#endif
    }
  }

  StateMap& states() { return states_; }

 private:
  StateMap states_;
};

// Protected by g_gmock_mutex.
MockObjectRegistry g_mock_object_registry;

// Maps a mock object to the reaction Google Mock should have when an
// uninteresting method is called.  Protected by g_gmock_mutex.
std::unordered_map<uintptr_t, internal::CallReaction>&
UninterestingCallReactionMap() {
  static auto* map = new std::unordered_map<uintptr_t, internal::CallReaction>;
  return *map;
}

// Sets the reaction Google Mock should have when an uninteresting
// method of the given mock object is called.
void SetReactionOnUninterestingCalls(uintptr_t mock_obj,
                                     internal::CallReaction reaction)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  UninterestingCallReactionMap()[mock_obj] = reaction;
}

}  // namespace

// Tells Google Mock to allow uninteresting calls on the given mock
// object.
void Mock::AllowUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kAllow);
}

// Tells Google Mock to warn the user about uninteresting calls on the
// given mock object.
void Mock::WarnUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kWarn);
}

// Tells Google Mock to fail uninteresting calls on the given mock
// object.
void Mock::FailUninterestingCalls(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  SetReactionOnUninterestingCalls(mock_obj, internal::kFail);
}

// Tells Google Mock the given mock object is being destroyed and its
// entry in the call-reaction table should be removed.
void Mock::UnregisterCallReaction(uintptr_t mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  UninterestingCallReactionMap().erase(static_cast<uintptr_t>(mock_obj));
}

// Returns the reaction Google Mock will have on uninteresting calls
// made on the given mock object.
internal::CallReaction Mock::GetReactionOnUninterestingCalls(
    const void* mock_obj) GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return (UninterestingCallReactionMap().count(
              reinterpret_cast<uintptr_t>(mock_obj)) == 0)
             ? internal::intToCallReaction(
                   GMOCK_FLAG_GET(default_mock_behavior))
             : UninterestingCallReactionMap()[reinterpret_cast<uintptr_t>(
                   mock_obj)];
}

// Tells Google Mock to ignore mock_obj when checking for leaked mock
// objects.
void Mock::AllowLeak(const void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].leakable = true;
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
bool Mock::VerifyAndClearExpectations(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies all expectations on the given mock object and clears its
// default actions and expectations.  Returns true if and only if the
// verification was successful.
bool Mock::VerifyAndClear(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  ClearDefaultActionsLocked(mock_obj);
  return VerifyAndClearExpectationsLocked(mock_obj);
}

// Verifies and clears all expectations on the given mock object.  If
// the expectations aren't satisfied, generates one or more Google
// Test non-fatal failures and returns false.
bool Mock::VerifyAndClearExpectationsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  if (g_mock_object_registry.states().count(mock_obj) == 0) {
    // No EXPECT_CALL() was set on the given mock object.
    return true;
  }

  // Verifies and clears the expectations on each mock method in the
  // given mock object.
  bool expectations_met = true;
  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    if (!(*it)->VerifyAndClearExpectationsLocked()) {
      expectations_met = false;
    }
  }

  // We don't clear the content of mockers, as they may still be
  // needed by ClearDefaultActionsLocked().
  return expectations_met;
}

bool Mock::IsNaggy(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kWarn;
}
bool Mock::IsNice(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kAllow;
}
bool Mock::IsStrict(void* mock_obj)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  return Mock::GetReactionOnUninterestingCalls(mock_obj) == internal::kFail;
}

// Registers a mock object and a mock method it owns.
void Mock::Register(const void* mock_obj,
                    internal::UntypedFunctionMockerBase* mocker)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  g_mock_object_registry.states()[mock_obj].function_mockers.insert(mocker);
}

// Tells Google Mock where in the source code mock_obj is used in an
// ON_CALL or EXPECT_CALL.  In case mock_obj is leaked, this
// information helps the user identify which object it is.
void Mock::RegisterUseByOnCallOrExpectCall(const void* mock_obj,
                                           const char* file, int line)
    GTEST_LOCK_EXCLUDED_(internal::g_gmock_mutex) {
  internal::MutexLock l(&internal::g_gmock_mutex);
  MockObjectState& state = g_mock_object_registry.states()[mock_obj];
  if (state.first_used_file == nullptr) {
    state.first_used_file = file;
    state.first_used_line = line;
    const TestInfo* const test_info =
        UnitTest::GetInstance()->current_test_info();
    if (test_info != nullptr) {
      state.first_used_test_suite = test_info->test_suite_name();
      state.first_used_test = test_info->name();
    }
  }
}

// Unregisters a mock method; removes the owning mock object from the
// registry when the last mock method associated with it has been
// unregistered.  This is called only in the destructor of
// FunctionMockerBase.
void Mock::UnregisterLocked(internal::UntypedFunctionMockerBase* mocker)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();
  for (MockObjectRegistry::StateMap::iterator it =
           g_mock_object_registry.states().begin();
       it != g_mock_object_registry.states().end(); ++it) {
    FunctionMockers& mockers = it->second.function_mockers;
    if (mockers.erase(mocker) > 0) {
      // mocker was in mockers and has been just removed.
      if (mockers.empty()) {
        g_mock_object_registry.states().erase(it);
      }
      return;
    }
  }
}

// Clears all ON_CALL()s set on the given mock object.
void Mock::ClearDefaultActionsLocked(void* mock_obj)
    GTEST_EXCLUSIVE_LOCK_REQUIRED_(internal::g_gmock_mutex) {
  internal::g_gmock_mutex.AssertHeld();

  if (g_mock_object_registry.states().count(mock_obj) == 0) {
    // No ON_CALL() was set on the given mock object.
    return;
  }

  // Clears the default actions for each mock method in the given mock
  // object.
  FunctionMockers& mockers =
      g_mock_object_registry.states()[mock_obj].function_mockers;
  for (FunctionMockers::const_iterator it = mockers.begin();
       it != mockers.end(); ++it) {
    (*it)->ClearDefaultActionsLocked();
  }

  // We don't clear the content of mockers, as they may still be
  // needed by VerifyAndClearExpectationsLocked().
}

Expectation::Expectation() = default;

Expectation::Expectation(
    const std::shared_ptr<internal::ExpectationBase>& an_expectation_base)
    : expectation_base_(an_expectation_base) {}

Expectation::~Expectation() = default;

// Adds an expectation to a sequence.
void Sequence::AddExpectation(const Expectation& expectation) const {
  if (*last_expectation_ != expectation) {
    if (last_expectation_->expectation_base() != nullptr) {
      expectation.expectation_base()->immediate_prerequisites_ +=
          *last_expectation_;
    }
    *last_expectation_ = expectation;
  }
}

// Creates the implicit sequence if there isn't one.
InSequence::InSequence() {
  if (internal::g_gmock_implicit_sequence.get() == nullptr) {
    internal::g_gmock_implicit_sequence.set(new Sequence);
    sequence_created_ = true;
  } else {
    sequence_created_ = false;
  }
}

// Deletes the implicit sequence if it was created by the constructor
// of this object.
InSequence::~InSequence() {
  if (sequence_created_) {
    delete internal::g_gmock_implicit_sequence.get();
    internal::g_gmock_implicit_sequence.set(nullptr);
  }
}

}  // namespace testing

#if defined(_MSC_VER) && (_MSC_VER == 1900)
GTEST_DISABLE_MSC_WARNINGS_POP_()  // 4800
#endif
// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <string>


GMOCK_DEFINE_bool_(catch_leaked_mocks, true,
                   "true if and only if Google Mock should report leaked "
                   "mock objects as failures.");

GMOCK_DEFINE_string_(verbose, testing::internal::kWarningVerbosity,
                     "Controls how verbose Google Mock's output is."
                     "  Valid values:\n"
                     "  info    - prints all messages.\n"
                     "  warning - prints warnings and errors.\n"
                     "  error   - prints errors only.");

GMOCK_DEFINE_int32_(default_mock_behavior, 1,
                    "Controls the default behavior of mocks."
                    "  Valid values:\n"
                    "  0 - by default, mocks act as NiceMocks.\n"
                    "  1 - by default, mocks act as NaggyMocks.\n"
                    "  2 - by default, mocks act as StrictMocks.");

namespace testing {
namespace internal {

// Parses a string as a command line flag.  The string should have the
// format "--gmock_flag=value".  When def_optional is true, the
// "=value" part can be omitted.
//
// Returns the value of the flag, or NULL if the parsing failed.
static const char* ParseGoogleMockFlagValue(const char* str,
                                            const char* flag_name,
                                            bool def_optional) {
  // str and flag must not be NULL.
  if (str == nullptr || flag_name == nullptr) return nullptr;

  // The flag must start with "--gmock_".
  const std::string flag_name_str = std::string("--gmock_") + flag_name;
  const size_t flag_name_len = flag_name_str.length();
  if (strncmp(str, flag_name_str.c_str(), flag_name_len) != 0) return nullptr;

  // Skips the flag name.
  const char* flag_end = str + flag_name_len;

  // When def_optional is true, it's OK to not have a "=value" part.
  if (def_optional && (flag_end[0] == '\0')) {
    return flag_end;
  }

  // If def_optional is true and there are more characters after the
  // flag name, or if def_optional is false, there must be a '=' after
  // the flag name.
  if (flag_end[0] != '=') return nullptr;

  // Returns the string after "=".
  return flag_end + 1;
}

// Parses a string for a Google Mock bool flag, in the form of
// "--gmock_flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                bool* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, true);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Converts the string value to a bool.
  *value = !(*value_str == '0' || *value_str == 'f' || *value_str == 'F');
  return true;
}

// Parses a string for a Google Mock string flag, in the form of
// "--gmock_flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
template <typename String>
static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                String* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, false);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Sets *value to the value of the flag.
  *value = value_str;
  return true;
}

static bool ParseGoogleMockFlag(const char* str, const char* flag_name,
                                int32_t* value) {
  // Gets the value of the flag as a string.
  const char* const value_str = ParseGoogleMockFlagValue(str, flag_name, true);

  // Aborts if the parsing failed.
  if (value_str == nullptr) return false;

  // Sets *value to the value of the flag.
  return ParseInt32(Message() << "The value of flag --" << flag_name, value_str,
                    value);
}

// The internal implementation of InitGoogleMock().
//
// The type parameter CharType can be instantiated to either char or
// wchar_t.
template <typename CharType>
void InitGoogleMockImpl(int* argc, CharType** argv) {
  // Makes sure Google Test is initialized.  InitGoogleTest() is
  // idempotent, so it's fine if the user has already called it.
  InitGoogleTest(argc, argv);
  if (*argc <= 0) return;

  for (int i = 1; i != *argc; i++) {
    const std::string arg_string = StreamableToString(argv[i]);
    const char* const arg = arg_string.c_str();

    // Do we see a Google Mock flag?
    bool found_gmock_flag = false;

#define GMOCK_INTERNAL_PARSE_FLAG(flag_name)            \
  if (!found_gmock_flag) {                              \
    auto value = GMOCK_FLAG_GET(flag_name);             \
    if (ParseGoogleMockFlag(arg, #flag_name, &value)) { \
      GMOCK_FLAG_SET(flag_name, value);                 \
      found_gmock_flag = true;                          \
    }                                                   \
  }

    GMOCK_INTERNAL_PARSE_FLAG(catch_leaked_mocks)
    GMOCK_INTERNAL_PARSE_FLAG(verbose)
    GMOCK_INTERNAL_PARSE_FLAG(default_mock_behavior)

    if (found_gmock_flag) {
      // Yes.  Shift the remainder of the argv list left by one.  Note
      // that argv has (*argc + 1) elements, the last one always being
      // NULL.  The following loop moves the trailing NULL element as
      // well.
      for (int j = i; j != *argc; j++) {
        argv[j] = argv[j + 1];
      }

      // Decrements the argument count.
      (*argc)--;

      // We also need to decrement the iterator as we just removed
      // an element.
      i--;
    }
  }
}

}  // namespace internal

// Initializes Google Mock.  This must be called before running the
// tests.  In particular, it parses a command line for the flags that
// Google Mock recognizes.  Whenever a Google Mock flag is seen, it is
// removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Mock flag variables are
// updated.
//
// Since Google Test is needed for Google Mock to work, this function
// also initializes Google Test and parses its flags, if that hasn't
// been done.
GTEST_API_ void InitGoogleMock(int* argc, char** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

// This overloaded version can be used in Windows programs compiled in
// UNICODE mode.
GTEST_API_ void InitGoogleMock(int* argc, wchar_t** argv) {
  internal::InitGoogleMockImpl(argc, argv);
}

// This overloaded version can be used on Arduino/embedded platforms where
// there is no argc/argv.
GTEST_API_ void InitGoogleMock() {
  // Since Arduino doesn't have a command line, fake out the argc/argv arguments
  int argc = 1;
  const auto arg0 = "dummy";
  char* argv0 = const_cast<char*>(arg0);
  char** argv = &argv0;

  internal::InitGoogleMockImpl(&argc, argv);
}

}  // namespace testing

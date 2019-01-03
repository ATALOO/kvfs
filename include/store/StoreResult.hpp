/*
 * Copyright (c) 2018 Afshin Sabahi. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 *
 *      Author: Afshin Sabahi
 *      File:   StoreResult.hpp
 */

/*
 *  Copyright (c) 2016-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <folly/Range.h>
#include <folly/FBString.h>
#include <string>

namespace folly {
    class IOBuf;
}

namespace kvfs {
    namespace store {

/*
 * StoreResult contains the result of a LocalStore lookup.
 *
 * RocksDB returns the data in a std::string (which is somewhat unfortunate,
 * since it gives us relatively poor memory management control).
 *
 * This class is a wrapper around the returned string, with a few benefits:
 * - It can also represent a "not found" result, so we can efficiently handle
 *   key lookups that are not present, without throwing an exception.
 * - It is move-only, so prevents us from ever unintentionally copying the
 *   string data.
 * - It provides APIs for creating IOBuf objects around the string result.
 */
        class StoreResult {
        public:
            /**
             * Construct an invalid StoreResult, representing a key that was not found.
             */
            StoreResult() = default;

            /**
             * Construct a StoreResult from a std::string.
             */
            explicit StoreResult(std::string&& data)
                    : valid_(true), data_(data) {}

            StoreResult(StoreResult&&) = default;
            StoreResult& operator=(StoreResult&&) = default;

            /**
             * Returns true if the value was found in the store,
             * or false if the key was not present.
             */
            bool isValid() const {
                return valid_;
            }

            /**
             * Get a reference to the std::string result.
             *
             * Throws std::domain_error if the key was not present in the store.
             *//*
            const std::string& asString() const {
                ensureValid();
                return data_.toStdString();
            }
*/
            /**
             * Get a reference to the std::string result.
             *
             * Throws std::domain_error if the key was not present in the store.
             */
            /*const folly::fbstring& asFBString() const {
                ensureValid();
                return data_;
            }*/

            /**
             * Get a ByteRange pointing to the result.
             *
             * Throws std::domain_error if the key was not present in the store.
             */
            folly::ByteRange bytes() const {
                ensureValid();
                return folly::StringPiece{data_};
            }

            /**
             * Get a StringPiece pointing to the result.
             *
             * Throws std::domain_error if the key was not present in the store.
             */
            folly::StringPiece piece() const {
                ensureValid();
                return folly::StringPiece{data_};
            }

            /**
             * Return an IOBuf that temporarily wraps this StoreResult.
             *
             * The IOBuf is unmanaged, and points to the string data contained in this
             * StoreResult.  It will be invalidated by any operation that invalidates the
             * StoreResult.
             */
            folly::IOBuf iobufWrapper() const;

            /**
             * Extract the std::string contained in this StoreResult.
             */
            std::string extractValue() {
                ensureValid();
                valid_ = false;
                return data_;
            }

            /**
             * Extract the data as an IOBuf.
             *
             * This will return a managed IOBuf, which will free the result data when
             * the last IOBuf clone is destroyed.
             *
             * This does require a memory allocation to move the stored std::string onto
             * the heap (but it just does a small allocation for the string object
             * itself, and not the string data).
             */
            folly::IOBuf extractIOBuf();

            /**
             * Throw an exception if this result is not valid
             * (i.e., if the key was not present in the store).
             */
            void ensureValid() const {
                if (!valid_) {
                    throwInvalidError();
                }
            }

        private:
            // Forbidden copy constructor and assignment operator
            StoreResult(StoreResult const&) = delete;
            StoreResult& operator=(StoreResult const&) = delete;

            [[noreturn]] void throwInvalidError() const;

            // Whether or not the result is value
            // If the key was not found in the store, valid_ will be false.
            bool valid_{false};

            // The string containing the data
            std::string data_;
        };
    } // namespace store
} // namespace kvfs
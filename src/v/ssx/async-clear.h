/*
 * Copyright 2022 Redpanda Data, Inc.
 *
 * Use of this software is governed by the Business Source License
 * included in the file licenses/BSL.md
 *
 * As of the Change Date specified in that file, in accordance with
 * the Business Source License, use of this software will be governed
 * by the Apache License, Version 2.0
 */

#pragma once

#include "seastarx.h"

#include <seastar/core/future.hh>
#include <seastar/coroutine/maybe_yield.hh>

#include <absl/container/flat_hash_map.h>

namespace ssx {

/**
 * Wrapping the async clear helper in a class is a workaround
 * for LLVM bug https://github.com/llvm/llvm-project/issues/49689
 *
 * Once we are on a version with the fix for #46989 backported,
 * this can be reduced to just a function.
 */
template<typename K, typename V>
class async_clear {
public:
    explicit async_clear(absl::flat_hash_map<K, V>& c)
      : _container(c) {}

    /**
     * For sufficiently large containers where the element destructors
     * do some work, it is problematic to spend a long time clearing
     * the container without yielding to the scheduler.
     *
     * This function yields every so often while erasing all elements
     * in a container.
     *
     * The type is specific to absl::flat_hash_map to avoid accidentially
     * using this function on types where repeatedly erasing from the start is
     * very expensive, like std::vector.
     */
    ss::future<> operator()() {
        // Below threshold_size, just call clear().
        // Otherwise yield to the scheduler every `threshold_size` elements
        constexpr size_t threshold_size = 100;

        if (_container.size() < threshold_size) {
            _container.clear();
            co_return;
        }

        size_t i = 0;
        while (_container.size()) {
            _container.erase(_container.begin());

            if (++i % threshold_size == 0) {
                co_await ss::coroutine::maybe_yield();
            }
        }
    }

    absl::flat_hash_map<K, V>& _container;
};

} // namespace ssx

//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Official repository: https://github.com/AlexAndDad/gateway
//

#pragma once

#include <openssl/sha.h>
#include <string>
#include <algorithm>
#include <array>
#include <string_view>
#include <openssl/bn.h>
#include <vector>
#include <cctype>
#include <iostream>

namespace McBot
{
    struct daft_hash_impl
    {
        daft_hash_impl()
            : ctx_{}
        {
            SHA1_Init(&ctx_);
        }

        daft_hash_impl(daft_hash_impl const&) = delete;
        daft_hash_impl(daft_hash_impl&&) = delete;
        daft_hash_impl& operator=(daft_hash_impl const&) = delete;
        daft_hash_impl& operator=(daft_hash_impl&&) = delete;
        ~daft_hash_impl() {}

        void update(void* data, size_t size) { SHA1_Update(&ctx_, data, size); }

        std::string finalise();

    private:
        SHA_CTX ctx_;
    };
}
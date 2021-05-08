#pragma once

#include <iostream>
#include <exception>

#include "Vector.h"
#include "Block.h"

namespace mcbot
{
    class CollisionException : public std::exception
    {
    private:
        mcbot::Block block;
        mcbot::Vector<double> location;
        std::string message;

    public:
        CollisionException(mcbot::Block block, mcbot::Vector<double> location) noexcept;

        virtual const char* what() const noexcept override;
    };
}


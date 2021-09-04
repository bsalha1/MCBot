#pragma once

#include <iostream>
#include <exception>

#include "Vector.h"
#include "Block.h"

namespace McBot
{
    class CollisionException : public std::exception
    {
    private:
        Block block;
        Vector<double> location;
        std::string message;

    public:
        CollisionException(Block block, Vector<double> location) noexcept;

        virtual const char* what() const noexcept override;
    };
}


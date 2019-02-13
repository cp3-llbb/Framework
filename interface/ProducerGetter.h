#pragma once

#include <string>

namespace Framework {
    class Producer;
};

class ProducerGetter {
    public:
        virtual ~ProducerGetter() noexcept(false); // to silence warning, and because subclass ExTreeMaker inherits a throwing constructor
        virtual const Framework::Producer& getProducer(const std::string& name) const = 0;
        virtual bool producerExists(const std::string& name) const = 0;
};

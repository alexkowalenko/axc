//
// AXC - C compiler
//
// Copyright © Alex Kowalenko 2025
//

#pragma once

#include <memory>

namespace {{namespace}} {
{% for class, members in type_items %}
class {{ class }}_;
using {{ class }} = std::shared_ptr<{{ class }}_>;
    {% endfor %}
 
template <typename T> class Visitor {
  public:
    virtual ~Visitor() = default;
    
    {% for class, members in type_items %}
    virtual T visit_{{ class }}({{ class }} const ast) = 0;
    {% endfor %}
};
}
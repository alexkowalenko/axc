//
// AXC - C compiler
//
// Copyright © Alex Kowalenko 2025
//

#pragma once

#include "visitor.h"

#include <memory>

#include "base.h"

{% for i in includes %}
#include "{{ i }}.h"
{% endfor %}

namespace {{namespace}} {

class {{ base_name }}_ : public Base, public std::enable_shared_from_this<{{ base_name }}_> {
  public:
    explicit {{ base_name }}_(Location const & loc) : Base(loc){};
    {% if members.__len__() is gt(0) %}
    {{ base_name }}_(Location const & loc  {% for field in members %}, {{ field[0] }} {{ field[1] }} {% endfor %})
      : Base(loc)  {% for field in members %}, {{ field[1] }}({{ field[1] }}) {% endfor %}{};
    {% endif %}
    ~{{ base_name }}_() override = default;

    {% for field in members %}
    {{ field[0] }} {{ field[1] }}{};
    {% endfor %}

    template <typename T> T accept(Visitor<T> *visitor)  {
        return visitor->visit_{{ base_name }}(shared_from_this());
    }
};

}
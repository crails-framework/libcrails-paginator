#include "paginator.hpp"
#include <algorithm>

using namespace Crails;

unsigned int Paginator::get_items_per_page() const
{
  unsigned int items_per_page = params["count"].defaults_to<unsigned short>(default_items_per_page);

  return std::min(items_per_page, max_items_per_page);
}

unsigned int Paginator::get_current_page() const
{
  return params["page"].defaults_to<unsigned short>(1);
}

unsigned int Paginator::get_current_offset() const
{
  return get_items_per_page() * (get_current_page() - 1);
}

void Paginator::decorate_view(SharedVars& vars, CountCallback get_total_items) const
{
  vars["with_paginator"] = enabled;
  if (enabled)
  {
    vars["page"]           = get_current_page();
    vars["items_per_page"] = get_items_per_page();
    vars["items_count"]    = get_total_items();
  }
}

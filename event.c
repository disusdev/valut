#include "event.h"
#include "xeno.h"

#define MAX_EVENT_LISTENERS 16

static event_t event;
static event_data_t event_funcs[MAX_EVENT_LISTENERS];
static uint32_t event_func_counter;

void
b_event_clear()
{
  platform_mem_set(&event, 0, sizeof(event_t));
  event_func_counter = 0;
}

int
b_event_register(event_kind_t kind,
                 event_func_t event_func)
{
  if (event_func_counter < MAX_EVENT_LISTENERS)
  {
    event_funcs[event_func_counter++] = (event_data_t){ kind, event_func };
    return 1;
  }
  return 0;
}

event_t*
b_event_handle_get()
{
  return &event;
}

void
b_event_dispatch(event_kind_t kind)
{
  b_event_dispatch_ext(kind, &event);
}

void
b_event_dispatch_ext(event_kind_t kind, event_t* e)
{
  e->kind = kind;
  int i;
  for(i = 0; i < event_func_counter; i++)
  {
    if (event_funcs[i].kind == kind)
    {
      event_funcs[i].func(e);
    }
  }
}

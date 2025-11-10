#ifndef __FRAMEGRAPH_H__
#define __FRAMEGRAPH_H__

#include <stdint.h>

void framegraph_size_set(int w, int h);
void framegraph_store(int idx, double dt, double frame);
void framegraph_draw(uint32_t* buffer, int count, uint32_t color);
float get_frame_avg_ratio();
int last_frame_pos(float ratio);

#endif
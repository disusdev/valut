#include "framegraph.h"
#include "nude.h"
#include "xeno.h"

static int width;
static int height;
static int graph_scale = 1;
static int* frames = 0;
static int frame_index = 0;

void
framegraph_size_set(int w, int h) {
    width = w;
    height = h;
    frame_index = w - 1;
    frames = (int*) x_alloc( sizeof(int) * w, 0 );
}

void
framegraph_store(int idx,
                 double dt,
                 double frame) {
    double ratio = dt / frame;
    int pos = height * ratio;
    int i;
    for (i = 1; i < width; i++) {
        frames[i - 1] = frames[i];
    }
    frames[frame_index] = pos;
}

void
framegraph_draw(uint32_t* buffer,
                int count,
                uint32_t color) {
    int i, x;
    for (i = 0; i < count; i++) {
        for (x = 0; x < width; x++) {
            int y = height - frames[x] * graph_scale;
            n_point_draw(buffer, x,  y, color);
        }
    }
}

float
get_frame_avg_ratio() {
    int sum = 0;
    int count = 0;
    int x;
    for (x = 0; x < width; x++) {
        if (frames[x] <= 0 || frames[x] >= (height - 1)) continue;
        sum += frames[x];
        count++;
    }
    if (count == 0) {
        return 0;
    }
    int avg = (width > 0) ? (sum / count) : 0;
    return (float)avg / height;
}

int
last_frame_pos(float ratio) {
    return width - height * ratio * graph_scale;
}

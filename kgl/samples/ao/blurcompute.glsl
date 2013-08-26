#version 430 core

#ifdef _COMPUTE_

#define TILE_SIZE 16
#define FILTER_SIZE 5
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;

uniform layout(binding = 0, r32f) coherent restrict readonly image2D in_Image;
uniform layout(binding = 1, rgba32f) coherent restrict writeonly image2D out_Image;

const ivec2 tileSize = ivec2(TILE_SIZE, TILE_SIZE);
const ivec2 filterOffset = ivec2(FILTER_SIZE/2, FILTER_SIZE/2);
const ivec2 neighborSize = tileSize + 2*filterOffset;
shared vec4 neighborhood[neighborSize.x][neighborSize.y];

ivec2 clampLocation(ivec2 input)
{
    return clamp(input, ivec2(0,0), imageSize(in_Image)); // assuming that in/out images are same size
}

float filter(uint x, uint y) {
    return 1.0 / (FILTER_SIZE * FILTER_SIZE);
}

void main()
{
    const ivec2 tile_id = ivec2(gl_WorkGroupID);
    const ivec2 thread_id = ivec2(gl_LocalInvocationID);
    const ivec2 pixel = tile_id * tileSize + thread_id;
    const uint x = thread_id.x; const uint y = thread_id.y;
    // first, copy into shared memory
    for (uint i = 0; i < neighborSize.y; i+=tileSize.y) {
        for (uint j = 0; j < neighborSize.x; j+=tileSize.x){
            if ((x+j) < neighborSize.x && (y+i) < neighborSize.y) {
                const ivec2 read_coord = clampLocation(ivec2(j,i) + pixel - filterOffset);
                neighborhood[x+j][y+i] = imageLoad(in_Image, read_coord);
            }
        }
    }
    memoryBarrierShared();
    barrier();
    vec4 total;
    // next, perform the convolution
    // position of the current pixel in shared memory is thread_id + filterOffset
    const ivec2 shared_pixel = thread_id + filterOffset; // guaranteed to be within shared memory bounds
    // so do filter lookups relative to that position

    for (uint i = 0; i < FILTER_SIZE; i++) {
        for (uint j = 0; j < FILTER_SIZE; j++) {
            const ivec2 offset = ivec2(j, i) - filterOffset; // go from -filtersize/2 to +filtersize/2 both ways
            total += neighborhood[shared_pixel.x + offset.x][shared_pixel.y + offset.y] * filter(j, i);
        }
    }
    // the ao data is only in red channel, but make it all channels just to look nice for now
    imageStore(out_Image, pixel, vec4(total.r, total.r, total.r, 1.0));
}

#endif

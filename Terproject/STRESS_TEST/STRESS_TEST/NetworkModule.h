#pragma once

void InitializeNetwork();
void GetPointCloud(int * size, float ** points);

extern std::atomic_int global_delay;
//extern std::atomic_int avg_delay;
extern volatile uint64_t avg_delay;
extern std::atomic_int active_clients;
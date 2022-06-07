#ifndef MSI_FANS_CPU_FANS_H
#define MSI_FANS_CPU_FANS_H

struct fan_config {
    unsigned char temps_low[7];
    unsigned char temps_silent[7];
    unsigned char temps_balanced[7];
    unsigned char temps_performance[7];
    unsigned char fan_speed_low[7];
    unsigned char fan_speed_silent[7];
    unsigned char fan_speed_balanced[7];
    unsigned char fan_speed_performance[7];
};

int mpc_cpu_fan_create(void);
void mpc_cpu_fan_destroy(void);

int mpc_gpu_fan_create(void);
void mpc_gpu_fan_destroy(void);

#endif
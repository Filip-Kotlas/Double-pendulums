#pragma once

#include "Pendulum_system.hpp"
#include "System.hpp"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cuda.h>
#include <cuda_runtime.h>

#include "CudaArray.hpp"

__global__ void cuda_aux_computation(const double* state, const double* k, double* aux, double tau, int dof, double factor);
__global__ void cuda_final_update(double* state, const double* k1, const double* k2, const double* k3, const double* k4, double tau, int dof);

class RungeKutta
{
    private:
        CudaArray k1;
        CudaArray k2;
        CudaArray k3;
        CudaArray k4;
        CudaArray aux;

        double time_step;
        double integration_step;
        System *current_system;

    
    public:
        void set_up(System *system, double time_step, double integration_step);
        void solve(double time_max);
        void integrate_step(double time_max);
};

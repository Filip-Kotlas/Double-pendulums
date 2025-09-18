#pragma once

#include "Pendulum_system.hpp"
#include "Pendulum_system.tpp"
#include "System.hpp"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <chrono>
#include <atomic>

template<typename Method, typename System, typename Real = double>//, typename = typename std::enable_if<std::is_floating_point<Real>::value>::type>
class Solver
{
public:
    Solver(System* system, Real time_step, Real integration_step, float* progress, std::atomic<bool>* cancel)
        : current_system_(system),
        time_step_(time_step),
        integration_step_(integration_step),
        progress_ptr_(progress),
        cancel_ptr_(cancel),
        method_(system) {}

     void solve(Real time_max) {
        int steps_count = std::ceil((time_max - this->current_system_->get_time())/time_step_);
        this->current_system_->record_state();
        
        for (int k = 0; k <= steps_count; k++) {
            if (cancel_ptr_ && cancel_ptr_->load())
                break;
            this->method_.integrate_step(this->current_system_, this->time_step_, this->integration_step_);
            this->current_system_->record_state();

            if (progress_ptr_) {
                *progress_ptr_ = float(k) / float(steps_count);
            }
        }
    }
        
private:
        Real time_step_;
        Real integration_step_;
        System *current_system_;
        float* progress_ptr_ = nullptr;
        std::atomic<bool>* cancel_ptr_ = nullptr;
        Method method_;

};

template<typename Real, typename System>
class RungeKutta_4
{
public:
    RungeKutta_4(System* system) {
        int dof = system->get_degrees_of_freedom();
        k1.resize(dof);
        k2.resize(dof);
        k3.resize(dof);
        k4.resize(dof);
        aux.resize(dof);
    }

    void integrate_step(System* system, Real time_step, Real integration_step) {
        Real start_time = system->get_time();
        while (integration_step <= start_time + time_step - system->get_time()) {
            Real tau = std::min(integration_step, start_time + time_step - system->get_time());

            // Computing k1
            system->get_right_hand_side(system->get_time(),
                                                system->get_state(),
                                                k1);
            
            // Computing k2
            for(int i = 0; i < system->get_degrees_of_freedom(); i++){
                aux[i] = system->get_state()[i] + 1.0/2 * tau * k1[i];
            }
            system->get_right_hand_side(system->get_time() + 1.0/2*tau,
                                                aux,
                                                k2);
            
            // Computing k3
            for(int i = 0; i < system->get_degrees_of_freedom(); i++){
                aux[i] = system->get_state()[i] + 1.0/2 * tau * k2[i];
            }
            system->get_right_hand_side(system->get_time() + 1.0/2*tau,
                                                aux,
                                                k3);

            // Computing k4
            for(int i = 0; i < system->get_degrees_of_freedom(); i++){
                aux[i] = system->get_state()[i] + tau * k3[i];
            }
            system->get_right_hand_side(system->get_time() + tau,
                                                aux,
                                                k4);

            for(int i = 0; i < system->get_degrees_of_freedom(); i++){
                system->get_state()[i] += 1.0/6 * tau * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]);
            }
            system->increase_time(tau);
        }
    }

private:
    std::vector<Real> k1;
    std::vector<Real> k2;
    std::vector<Real> k3;
    std::vector<Real> k4;
    std::vector<Real> aux;

};

template<typename Real, typename System>
struct EulerMethod {
    static void integrate_step(System* system, Real time_step, Real integration_step) {
        Real start_time = system->get_time();
        std::vector<Real> rhs(system->get_degrees_of_freedom());

        while (integration_step <= start_time + time_step - system->get_time()) {
            Real tau = std::min(integration_step, start_time + time_step - system->get_time());

            system->get_right_hand_side(system->get_time(), system->get_state(), rhs);

            for (int i = 0; i < system->get_degrees_of_freedom(); i++) {
                system->get_state()[i] += tau * rhs[i];
            }
            system->increase_time(tau);
        }
    }
};
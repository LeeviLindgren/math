#include<stan/math/prim/meta.hpp>
#include<stan/math/rev/meta.hpp>
#include<stan/math.hpp>
#include<iostream>

template<typename T, stan::require_container_t<T>* = nullptr, 
         stan::require_st_arithmetic<T>* = nullptr>
void print(const T& x) {
    std::cout << "Container of arithmetics" << '\n';
    for (int i = 0; i < x.size(); i++) {
        std::cout << x[i] << ' ';
    }
    std::cout << '\n';
}

template<typename T, stan::require_container_t<T>* = nullptr, 
         stan::require_st_var<T>* = nullptr>
void print(const T& x) {
    std::cout << "Container of vars" << '\n';
    for (int i = 0; i < x.size(); i++) {
        std::cout << x[i].val() << ' ';
    }
    std::cout << '\n';
}

/*
Declare vector of stan vars (reverse mode autodiff specialization) and
a Eigen vector of doubles. Use Stan's internal type traits to ensure a right
specialization is called.

Use following type traits:
require_container_t<T> -> ensure that type is either eigen matrix or std::vector
require_st_arithmetic<T> -> ensure that the scalar type of the container is arithmetic (stan::math::var is not)
require_st_var<T> -> ensure that the scalar type of the container is stan::math::var
*/
int main() {
    std::vector<stan::math::var> var_v;
    Eigen::Vector3d v { 1, 2, 3};

    stan::math::var v1 {4};
    stan::math::var v2 {5};
    stan::math::var v3 {6};

    var_v.push_back(v1);
    var_v.push_back(v2);
    var_v.push_back(v3);

    print(v);
    print(var_v);
    return 0;
}
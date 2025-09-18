#pragma once
constexpr double pow_int(double base, int exp) {
    return (exp == 0) ? 1.0 : base * pow_int(base, exp - 1);
}

template<int N>
struct Pi {
    static constexpr double value = 1.0/pow_int(16, N)*( 4.0/(8*N + 1)
                                                        - 2.0/(8*N + 4)
                                                        - 1.0/(8*N + 5)
                                                        - 1.0/(8*N + 6))
                                + Pi<N-1>::value;
};

template<>
struct Pi<-1> {
    static constexpr double value = 0;
};

template<int N>
constexpr double PI = Pi<N>::value;


template<typename T>
struct AngleConverter {
    static constexpr T rad_to_deg = static_cast<T>(180.0) / PI<6>;
    static constexpr T deg_to_rad = PI<6> / static_cast<T>(180.0);
};
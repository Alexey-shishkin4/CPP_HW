#include <iostream>


template<int N, int D>
struct IsPrimeHelper {
    static const bool value = (N % D != 0) && IsPrimeHelper<N, D - 1>::value;
};


template<int N>
struct IsPrimeHelper<N, 1> {
    static const bool value = true;
};


template<int N>
struct IsPrime {
    static const bool value = IsPrimeHelper<N, N - 1>::value;
};


template<int Count, int Current, int Found = 0>
struct NthPrime {
    static const int value = NthPrime<Count, Current + 1, Found + (IsPrime<Current>::value ? 1 : 0)>::value;
};


template<int Count, int Current>
struct NthPrime<Count, Current, Count> {
    static const int value = Current - 1;
};


int main() {
    constexpr int N = 10;
    std::cout << "The " << N << "th prime is: " << NthPrime<N, 2>::value << "\n";
}

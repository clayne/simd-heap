#include <vector>
#include <algorithm>
#include <random>
#include <cstdint>

#include "push_heap.h"
#include "push_heap_avx512.h"

using Type = uint32_t;

namespace {
    void push_heap_std_wrapper(Type* start, Type* end) {
        std::push_heap(start, end);
    }

    void push_heap_scalar_wrapper(Type* start, Type* end) {
        push_heap_scalar(start, end, std::less<Type>());
    }
}

class Application {

    const size_t size = 65536;
    const size_t valid_heap_size = 32768 + 1;
    std::vector<Type> heap;

public:
    Application() {
        push_heap_avx512_setup();
    }

    bool run() {
        bool all_ok = true;

        printf("Testing for input size %lu\n", size);
        std::vector<Type> input_data = generate_random_data(size);
        std::make_heap(input_data.begin(), input_data.begin() + valid_heap_size);

        heap = input_data;
        all_ok = test("std::push_heap",     push_heap_std_wrapper) and all_ok;

        heap = input_data;
        all_ok = test("scalar push_heap",   push_heap_scalar_wrapper) and all_ok;

        heap = input_data;
        all_ok = test("AVX512 push_heap",   push_heap_avx512) and all_ok;

        return all_ok;
    }

private:
    template <typename Function>
    bool test(const std::string& name, Function function) {
        printf("testing function %s...", name.c_str()); fflush(stdout);

        for (size_t i=valid_heap_size; i <= heap.size(); i++)
            function(heap.data(), heap.data() + i);

        const bool is_heap = std::is_heap(heap.data(), heap.data() + heap.size());
        printf("%s\n", (is_heap) ? "OK" : "failed");

        return is_heap;
    }

    std::vector<Type> generate_random_data(size_t size) {
        std::random_device rd;
        std::mt19937 eng(rd());
        eng.seed(0);

        std::uniform_int_distribution<Type> random(0, 0xffffffff);

        std::vector<Type> heap;
        heap.reserve(size);
        for (size_t i=0; i < size; i++) {
            heap.push_back(random(eng));
        }

        return heap;
    }
};

int main() {
    Application app{};

    return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
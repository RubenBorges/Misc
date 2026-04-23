#include <mpi.h>
#include <sycl/sycl.hpp>
#include <tbb/parallel_for.h>
#include <mkl.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    // --- MPI ---
    MPI_Init(&argc, &argv);
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_rank == 0)
        std::cout << "[MPI] Hello from rank " << world_rank << " of " << world_size << std::endl;

    // --- TBB ---
    constexpr int N = 8;
    std::vector<int> tbb_data(N);
    tbb::parallel_for(0, N, [&](int i) { tbb_data[i] = i * 2; });
    if (world_rank == 0) {
        std::cout << "[TBB] parallel_for: ";
        for (auto v : tbb_data) std::cout << v << " ";
        std::cout << std::endl;
    }

    // --- SYCL ---
    std::vector<int> sycl_data(N, 1);
    {
        sycl::queue q;
        sycl::buffer<int, 1> buf(sycl_data.data(), sycl::range<1>(N));
        q.submit([&](sycl::handler& h) {
            auto acc = buf.get_access<sycl::access::mode::read_write>(h);
            h.parallel_for(sycl::range<1>(N), [=](sycl::id<1> i) {
                acc[i] += 10;
            });
        });
    }
    if (world_rank == 0) {
        std::cout << "[SYCL] results: ";
        for (auto v : sycl_data) std::cout << v << " ";
        std::cout << std::endl;
    }

    // --- MKL ---
    std::vector<float> x(N, 1.0f), y(N, 2.0f);
    cblas_saxpy(N, 3.0f, x.data(), 1, y.data(), 1); // y = 3*x + y
    if (world_rank == 0) {
        std::cout << "[MKL] cblas_saxpy: ";
        for (auto v : y) std::cout << v << " ";
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
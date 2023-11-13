#include <iostream>
#include <cstdlib>
#include <cmath>
#include <mpi.h>
#include <cstddef>

constexpr int DOUBLE_MAX = 10;
struct CustomData {
  int n_values;
  double values[DOUBLE_MAX];
};

int main(int argc, char **argv) {
  
  MPI_Init(&argc, &argv);

  int rank, size;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  constexpr int n_structure_per_process = 5; // M = 5

  // Inicialização do gerador de números aleatórios
  srand(rank * 10);
  
  // Criação do conjunto de dados
  CustomData data[n_structure_per_process];

  // Geração dos dados
  for (int i=0; i < n_structure_per_process; ++i) {
    // gerar números aleatórios
    data[i].n_values = rand() % DOUBLE_MAX + 1; 
    for (int j=0; j < DOUBLE_MAX; ++j)
      data[i].values[j] = (j < data[i].n_values ? (double)rand() / (double)RAND_MAX : 0.0);
  }

  // 1- Aqui, criar todas as propriedades para chamar MPI_Type_create_struct
  MPI_Aint displacements[2]  = {};
  int block_lengths[2]  = {1, DOUBLE_MAX};
  MPI_Datatype types[2] = {MPI_INT, MPI_DOUBLE};
  MPI_Datatype custom_dt;

  // 2- Criar o tipo e registrá-lo
  MPI_Type_create_struct(2, block_lengths, displacements, types, &custom_dt);
  MPI_Type_commit(&custom_dt);

  // Reunindo os dados
  CustomData *gathered_data = nullptr;

  if (rank == 0)
    gathered_data = new CustomData[n_structure_per_process * size];
  
  MPI_Gather(data, n_structure_per_process, custom_dt, gathered_data, n_structure_per_process, custom_dt, 0, MPI_COMM_WORLD);

  // imprimindo
  if (rank == 0) {
    for (int i=0; i < size; ++i) {
      for (int j=0; j < n_structure_per_process; ++j) {
        int data_id = i * n_structure_per_process + j; // Índice linear

        std::cout << "Estrutura de dados " << data_id << " : [";
        int N = gathered_data[data_id].n_values;
        
        for (int k=0; k < N; ++k) {
          std::cout << gathered_data[data_id].values[k] << (k == N-1 ? "]" : "; ");
        }
        std::cout << std::endl;
      }
    }
  }

  MPI_Type_free(&custom_dt); // Liberar o tipo de dado criado
  if (rank == 0)
    delete[] gathered_data;

  MPI_Finalize();
  
  return 0;
}

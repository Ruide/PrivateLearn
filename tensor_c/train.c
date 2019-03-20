// Example of training the model created by model.py using the TensorFlow C API.
//
// To run use c.sh in this directory.

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <tensorflow/c/c_api.h>

typedef struct model_t {
  TF_Graph* graph;
  TF_Session* session;
  TF_Status* status;

  TF_Output input, target, output;

  TF_Operation *init_op, *train_op, *save_op, *restore_op;
  TF_Output checkpoint_file;
} model_t;

int ModelCreate(model_t* model, const char* graph_def_filename);
void ModelDestroy(model_t* model);
int ModelInit(model_t* model);
int ModelPredict(model_t* model, float* batch, int batch_size);
int ModelRunTrainStep(model_t* model);
enum SaveOrRestore { SAVE, RESTORE };
int ModelCheckpoint(model_t* model, const char* checkpoint_prefix, int type);

int Okay(TF_Status* status);
TF_Buffer* ReadFile(const char* filename);
TF_Tensor* ScalarStringTensor(const char* data, TF_Status* status);
int DirectoryExists(const char* dirname);

int main(int argc, char** argv) {
  const char* graph_def_filename = "graph.pb";
  const char* checkpoint_prefix = "./checkpoints/checkpoint";
  int restore = DirectoryExists("checkpoints");

  model_t model;
  printf("Loading graph\n");
  if (!ModelCreate(&model, graph_def_filename)) return 1;
  if (restore) {
    printf(
        "Restoring weights from checkpoint (remove the checkpoints directory "
        "to reset)\n");
    if (!ModelCheckpoint(&model, checkpoint_prefix, RESTORE)) return 1;
  } else {
    printf("Initializing model weights\n");
    if (!ModelInit(&model)) return 1;
  }

  float testdata[3] = {1.0, 2.0, 3.0};
  printf("Initial predictions\n");
  if (!ModelPredict(&model, &testdata[0], 3)) return 1;

  printf("Training for a few steps\n");
  for (int i = 0; i < 200; ++i) {
    if (!ModelRunTrainStep(&model)) return 1;
  }

  printf("Updated predictions\n");
  if (!ModelPredict(&model, &testdata[0], 3)) return 1;

  printf("Saving checkpoint\n");
  if (!ModelCheckpoint(&model, checkpoint_prefix, SAVE)) return 1;

  ModelDestroy(&model);
}

int ModelCreate(model_t* model, const char* graph_def_filename) {
  model->status = TF_NewStatus();
  model->graph = TF_NewGraph();

  {
    // Create the session.
    TF_SessionOptions* opts = TF_NewSessionOptions();
    model->session = TF_NewSession(model->graph, opts, model->status);
    TF_DeleteSessionOptions(opts);
    if (!Okay(model->status)) return 0;
  }

  TF_Graph* g = model->graph;

  {
    // Import the graph.
    TF_Buffer* graph_def = ReadFile(graph_def_filename);
    if (graph_def == NULL) return 0;
    printf("Read GraphDef of %zu bytes\n", graph_def->length);
    TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();
    TF_GraphImportGraphDef(g, graph_def, opts, model->status);
    TF_DeleteImportGraphDefOptions(opts);
    TF_DeleteBuffer(graph_def);
    if (!Okay(model->status)) return 0;
  }

  // Handles to the interesting operations in the graph.
  model->input.oper = TF_GraphOperationByName(g, "input");
  model->input.index = 0;
  model->target.oper = TF_GraphOperationByName(g, "target");
  model->target.index = 0;
  model->output.oper = TF_GraphOperationByName(g, "output");
  model->output.index = 0;

  model->init_op = TF_GraphOperationByName(g, "init");
  model->train_op = TF_GraphOperationByName(g, "train");
  model->save_op = TF_GraphOperationByName(g, "save/control_dependency");
  model->restore_op = TF_GraphOperationByName(g, "save/restore_all");

  model->checkpoint_file.oper = TF_GraphOperationByName(g, "save/Const");
  model->checkpoint_file.index = 0;
  return 1;
}

void ModelDestroy(model_t* model) {
  TF_DeleteSession(model->session, model->status);
  Okay(model->status);
  TF_DeleteGraph(model->graph);
  TF_DeleteStatus(model->status);
}

int ModelInit(model_t* model) {
  const TF_Operation* init_op[1] = {model->init_op};
  TF_SessionRun(model->session, NULL,
                /* No inputs */
                NULL, NULL, 0,
                /* No outputs */
                NULL, NULL, 0,
                /* Just the init operation */
                init_op, 1,
                /* No metadata */
                NULL, model->status);
  return Okay(model->status);
}
int ModelCheckpoint(model_t* model, const char* checkpoint_prefix, int type) {
  TF_Tensor* t = ScalarStringTensor(checkpoint_prefix, model->status);
  if (!Okay(model->status)) {
    TF_DeleteTensor(t);
    return 0;
  }
  TF_Output inputs[1] = {model->checkpoint_file};
  TF_Tensor* input_values[1] = {t};
  const TF_Operation* op[1] = {type == SAVE ? model->save_op
                                            : model->restore_op};
  TF_SessionRun(model->session, NULL, inputs, input_values, 1,
                /* No outputs */
                NULL, NULL, 0,
                /* The operation */
                op, 1, NULL, model->status);
  TF_DeleteTensor(t);
  return Okay(model->status);
}

int ModelPredict(model_t* model, float* batch, int batch_size) {
  // batch consists of 1x1 matrices.
  const int64_t dims[3] = {batch_size, 1, 1};
  const size_t nbytes = batch_size * sizeof(float);
  TF_Tensor* t = TF_AllocateTensor(TF_FLOAT, dims, 3, nbytes);
  memcpy(TF_TensorData(t), batch, nbytes);

  TF_Output inputs[1] = {model->input};
  TF_Tensor* input_values[1] = {t};
  TF_Output outputs[1] = {model->output};
  TF_Tensor* output_values[1] = {NULL};

  TF_SessionRun(model->session, NULL, inputs, input_values, 1, outputs,
                output_values, 1,
                /* No target operations to run */
                NULL, 0, NULL, model->status);
  TF_DeleteTensor(t);
  if (!Okay(model->status)) return 0;

  if (TF_TensorByteSize(output_values[0]) != nbytes) {
    fprintf(stderr,
            "ERROR: Expected predictions tensor to have %zu bytes, has %zu\n",
            nbytes, TF_TensorByteSize(output_values[0]));
    TF_DeleteTensor(output_values[0]);
    return 0;
  }
  float* predictions = (float*)malloc(nbytes);
  memcpy(predictions, TF_TensorData(output_values[0]), nbytes);
  TF_DeleteTensor(output_values[0]);

  printf("Predictions:\n");
  for (int i = 0; i < batch_size; ++i) {
    printf("\t x = %f, predicted y = %f\n", batch[i], predictions[i]);
  }
  free(predictions);
  return 1;
}

void NextBatchForTraining(TF_Tensor** inputs_tensor,
                          TF_Tensor** targets_tensor) {
#define BATCH_SIZE 10
  float inputs[BATCH_SIZE] = {0};
  float targets[BATCH_SIZE] = {0};
  for (int i = 0; i < BATCH_SIZE; ++i) {
    inputs[i] = (float)rand() / (float)RAND_MAX;
    targets[i] = 3.0 * inputs[i] + 2.0;
  }
  const int64_t dims[] = {BATCH_SIZE, 1, 1};
  size_t nbytes = BATCH_SIZE * sizeof(float);
  *inputs_tensor = TF_AllocateTensor(TF_FLOAT, dims, 3, nbytes);
  *targets_tensor = TF_AllocateTensor(TF_FLOAT, dims, 3, nbytes);
  memcpy(TF_TensorData(*inputs_tensor), inputs, nbytes);
  memcpy(TF_TensorData(*targets_tensor), targets, nbytes);
#undef BATCH_SIZE
}

int ModelRunTrainStep(model_t* model) {
  TF_Tensor *x, *y;
  NextBatchForTraining(&x, &y);
  TF_Output inputs[2] = {model->input, model->target};
  TF_Tensor* input_values[2] = {x, y};
  const TF_Operation* train_op[1] = {model->train_op};
  TF_SessionRun(model->session, NULL, inputs, input_values, 2,
                /* No outputs */
                NULL, NULL, 0, train_op, 1, NULL, model->status);
  TF_DeleteTensor(x);
  TF_DeleteTensor(y);
  return Okay(model->status);
}

int Okay(TF_Status* status) {
  if (TF_GetCode(status) != TF_OK) {
    fprintf(stderr, "ERROR: %s\n", TF_Message(status));
    return 0;
  }
  return 1;
}

TF_Buffer* ReadFile(const char* filename) {
  int fd = open(filename, 0);
  if (fd < 0) {
    perror("failed to open file: ");
    return NULL;
  }
  struct stat stat;
  if (fstat(fd, &stat) != 0) {
    perror("failed to read file: ");
    return NULL;
  }
  char* data = (char*)malloc(stat.st_size);
  ssize_t nread = read(fd, data, stat.st_size);
  if (nread < 0) {
    perror("failed to read file: ");
    free(data);
    return NULL;
  }
  if (nread != stat.st_size) {
    fprintf(stderr, "read %zd bytes, expected to read %zd\n", nread,
            stat.st_size);
    free(data);
    return NULL;
  }
  TF_Buffer* ret = TF_NewBufferFromString(data, stat.st_size);
  free(data);
  return ret;
}

TF_Tensor* ScalarStringTensor(const char* str, TF_Status* status) {
  size_t nbytes = 8 + TF_StringEncodedSize(strlen(str));
  TF_Tensor* t = TF_AllocateTensor(TF_STRING, NULL, 0, nbytes);
  void* data = TF_TensorData(t);
  memset(data, 0, 8);  // 8-byte offset of first string.
  TF_StringEncode(str, strlen(str), data + 8, nbytes - 8, status);
  return t;
}

int DirectoryExists(const char* dirname) {
  struct stat buf;
  return stat(dirname, &buf) == 0;
}


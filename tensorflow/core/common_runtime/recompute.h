#include <condition_variable>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>

#include "tensorflow/core/platform/types.h"

namespace tensorflow {

class TensorBuffer;
class Tensor;
class RecomputeHelper {
 public:
  static RecomputeHelper* GlobalRecomputeHelper() {
    static RecomputeHelper* helper = new RecomputeHelper;
    return helper;
  }
  typedef std::function<void()> RecomputeDoneCallback;
  typedef std::function<void(const std::string&, const std::vector<std::string>&, RecomputeDoneCallback)> RecomputeCall;
  void RecordTensorAccess(const std::string& tensor_name, const uint64 time_); 
  void RecordTensorBuffer(const std::string& tensor_name, Tensor* tensor);
  void RecordRecomputeCall(const std::string& tensor_name, RecomputeCall call);
  void RecomputeTensor(const std::string& tensor_name);
  void LoadRecomputePolicy();
  void DeleteMemory(const std::string& tensor_name);
  void IncrementUsingCount(const std::string& tensor_name);
  void DecrementUsingCount(const std::string& tensor_name);
 private:
  RecomputeHelper() { LoadRecomputePolicy(); }
  typedef std::pair<std::shared_ptr<std::condition_variable>, std::shared_ptr<std::mutex>> condition_variable_and_mutex;
  enum DataStatus {
    IN,
    OUT,
    RECOMPUTING
  };
  struct Params {
    condition_variable_and_mutex cv_mu;
    volatile int data_ready;
    std::string target_tensor;
    std::vector<std::string> feed_tensors;
    TensorBuffer* buf;
    volatile int using_count;
    bool then_delete;
  };

  struct TriggerInfo {
    std::string tensor_name;
    int access_count;
    int total_access_count;
    int delete_trigger_count; // delete itself
    std::vector<std::vector<std::string>> recompute_tensors;
  };

  std::unordered_map<std::string, Params> tensor_recompute_params_;
  std::unordered_map<std::string, RecomputeCall> recompute_calls;
  std::unordered_map<std::string, TriggerInfo> triggers_;
  std::mutex mu_;
};
}

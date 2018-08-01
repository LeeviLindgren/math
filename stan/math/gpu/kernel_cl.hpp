#ifndef STAN_MATH_GPU_KERNEL_CL_HPP
#define STAN_MATH_GPU_KERNEL_CL_HPP
#ifdef STAN_OPENCL

#include <stan/math/gpu/opencl_context.hpp>
#include <stan/math/gpu/constants.hpp>
#include <map>
#include <string>
#include <vector>

namespace stan {
namespace math {

class kernel_cl_base {
  friend class kernel_cl;

private:
  const char* copy_matrix_kernel =
#include <stan/math/gpu/kernels/copy_matrix_kernel.cl>
      ;  // NOLINT
  const char* transpose_matrix_kernel =
#include <stan/math/gpu/kernels/transpose_matrix_kernel.cl>
      ;  // NOLINT
  const char* zeros_matrix_kernel =
#include <stan/math/gpu/kernels/zeros_matrix_kernel.cl>
      ;  // NOLINT
  const char* identity_matrix_kernel =
#include <stan/math/gpu/kernels/identity_matrix_kernel.cl>
      ;  // NOLINT
  const char* copy_triangular_matrix_kernel =
#include <stan/math/gpu/kernels/copy_triangular_matrix_kernel.cl>
      ;  // NOLINT
  const char* copy_triangular_transposed_matrix_kernel =
#include <stan/math/gpu/kernels/triangular_transpose_kernel.cl>
      ;  // NOLINT
  const char* copy_submatrix_kernel =
#include <stan/math/gpu/kernels/sub_block_kernel.cl>
      ;  // NOLINT
  const char* check_nan_kernel =
#include <stan/math/gpu/kernels/check_nan_kernel.cl>
      ;  // NOLINT
  const char* check_diagonal_zeros_kernel =
#include <stan/math/gpu/kernels/check_diagonal_zeros_kernel.cl>
      ;  // NOLINT
  const char* check_symmetric_kernel =
#include <stan/math/gpu/kernels/check_symmetric_kernel.cl>
      ;  // NOLINT
  const char* subtract_symmetric_kernel =
#include <stan/math/gpu/kernels/subtract_matrix_kernel.cl>
      ;  // NOLINT
  const char* add_symmetric_kernel =
#include <stan/math/gpu/kernels/add_matrix_kernel.cl>
      ;  // NOLINT

protected:
  typedef std::map<const char*, int> map_base_opts;
  const map_base_opts base_opts = {
  {"LOWER", gpu::Lower},
  {"UPPER", gpu::Upper},
  {"ENTIRE", gpu::Entire},
  {"UPPER_TO_LOWER", gpu::UpperToLower},
  {"LOWER_TO_UPPER", gpu::LowerToUpper}};

  /** Holds meta information about a kernel.
   * @param exists a bool to identify whether a kernel has been compiled.
   * @param group The name of the compilation group for the kernel.
   * @param opts The compile time options for the kernel.
   * @param code The source code for the kernel.
   */
  struct kernel_meta_info {
    bool exists;
    const char* group;
    std::vector<const char*> opts;
    const char* raw_code;
  };
  /**
   * Map of a kernel name (first) and it's meta information (second).
   */
  typedef std::map<const char*, kernel_meta_info> map_kernel_info;
  const map_kernel_info kernel_info = {
  {"dummy", { false, "timing", {},
     "__kernel void dummy(__global const int* foo) { };"}},
  {"dummy2", {false, "timing", {},
     "__kernel void dummy2(__global const int* foo) { };"}},
  {"copy", {false, "basic_matrix", {}, copy_matrix_kernel}},
  {"transpose", {false, "basic_matrix", {}, transpose_matrix_kernel}},
  {"zeros", {false, "basic_matrix",
     {"LOWER", "UPPER", "ENTIRE"}, zeros_matrix_kernel}},
  {"identity", {false, "basic_matrix", {}, identity_matrix_kernel}},
  {"copy_triangular", {false, "basic_matrix", {},
     copy_triangular_matrix_kernel}},
  {"copy_triangular_transposed", {false, "basic_matrix",
     {"LOWER_TO_UPPER", "UPPER_TO_LOWER"},
     copy_triangular_transposed_matrix_kernel}},
  {"copy_submatrix", {false, "basic_matrix", {}, copy_submatrix_kernel}},
  {"add", {false, "basic_matrix", {}, add_symmetric_kernel}},
  {"subtract", {false, "basic_matrix", {}, subtract_symmetric_kernel}},
  {"is_nan", {false, "check", {""}, check_nan_kernel}},
  {"is_zero_on_diagonal", {false, "check", {}, check_diagonal_zeros_kernel}},
  {"is_symmetric", {false, "check", {}, check_symmetric_kernel}}};
  typedef std::map<const char*, cl::Kernel> map_kernel;
  map_kernel kernels;           // The compiled kernels

  static kernel_cl_base& getInstance() {
    static kernel_cl_base instance_;
    return instance_;
  }
  kernel_cl_base() = default;
  kernel_cl_base(kernel_cl_base const&) = delete;
  void operator=(kernel_cl_base const&) = delete;
};

class kernel_cl {
public:
  cl::Kernel compiled_;
  /**
   * Compiles all the kernels in the specified group. The side effect of this
   *  method places all compiled kernels for a group inside of <code> kernels
   *  </code>.
   *
   * @param kernel_name[in] The kernel name.
   *
   * @throw std::system_error if there are compilation errors
   * when compiling the specified kernel group's source code
   * or std::domain_error if the kernel with the specified
   * name does not exist
   */
  inline void compile_kernel_group(const char* kernel_name) {
    std::string kernel_opts = "";
    std::string kernel_source = "";
    if (this->kernel_info().count(kernel_name) == 0) {
      // throws if the kernel does not exist
      domain_error("compiling kernels", kernel_name,
                   " kernel does not exist", "");
    }
    const char* kernel_group = this->kernel_info()[kernel_name].group;
    for (auto kern : this->kernel_info()) {
      if (strcmp(kern.second.group, kernel_group) == 0) {
        kernel_source += kern.second.raw_code;
        for (auto comp_opts : kern.second.opts) {
          if (strcmp(comp_opts, "") != 0) {
            kernel_opts += std::string(" -D") + comp_opts + "=" +
             std::to_string(this->base_options()[comp_opts]);
          }
        }
      }
    }

    try {
      cl::Program::Sources source(
          1,
          std::make_pair(kernel_source.c_str(), strlen(kernel_source.c_str())));
      cl::Program program_ = cl::Program(opencl_context.context(), source);
      program_.build({opencl_context.device()}, kernel_opts.c_str());

      cl_int err = CL_SUCCESS;
      // Iterate over the kernel list and get all the kernels from this group
      // and mark them as compiled.
      for (auto kern : this->kernel_info()) {
        if (strcmp(kern.second.group, kernel_group) == 0) {
          kernel_cl_base::getInstance().kernels[(kern.first)]
              = cl::Kernel(program_, kern.first, &err);
          kern.second.exists = true;
        }
      }
    } catch (const cl::Error& e) {
      check_opencl_error("Kernel Compilation", e);
    }
  }
  /**
   * Returns the kernel specified in kernel_name.
   * If the kernel has not yet been compiled, the kernel group is compiled
   * first.
   *
   * @brief Passing the name of a kernel compiles all kernels in the same group
   * as the selected kernel.
   * OpenCL kernels are compiled JIT, instead of compiling each kernel
   * individually this function will compile all kernels
   * in a predefined group. Groupings are made such that kernels commonly
   * called with one another will be compiled at the same time. For example,
   * An arithmetic group of kernels compiled together could contain the kernels
   * for <code> add() </code>, <code> subtract() </code>,
   * and <code> multiply() </code>. This function will only return the kernel
   * which was called, but when a user asks for a kernel within the group those
   * kernels will already be compiled.
   *
   * @param[in] kernel_name The kernel name.
   */
   explicit kernel_cl(const char* kernel_name) {
    // Compile the kernel group and return the kernel
    if (!this->kernel_info()[kernel_name].exists) {
      this->compile_kernel_group(kernel_name);
    }
    compiled_ = kernel_cl_base::getInstance().kernels[(kernel_name)];
  }
  /**
   * Terminating function for recursively setting arguments in an OpenCL kernel.
   *
   * @param k An OpenCL kernel.
   * @param i The <code>i</code>th argument to the kernel.
   * @note This function definition serves to end the recursive call for
   * <code>set_kernel_args()</code>
   */
  inline void recursive_args(cl::Kernel& k, int i) {}

  /**
   * Used in <code>set_kernel_args()</code> to add arguments to an OpenCL
   * kernel.
   *
   * @param kernel An OpenCL kernel.
   * @param i the position of the argument to the OpenCL kernel.
   * @param first_arg The first argument to go into the OpenCL kernel.
   * @param extra_args The remaining arguments to go into the OpenCL kernel.
   * @tparam T the type of <code>first_arg</code>.
   * @tparam Args The types of <code>extra_args</code>.
   * @note Comes from:
   * simpleopencl.blogspot.com/2013/04/calling-kernels-with-large-number-of.html
   */
  template <typename T, typename... Args>
  inline void recursive_args(cl::Kernel& kernel, int i,
                                    const T& first_arg,
                                    const Args&... extra_args) {
    kernel.setArg(i, first_arg);
    this->recursive_args(kernel, i + 1, extra_args...);
  }

  /**
   * Adds arguments to an OpenCL kernel.
   *
   * @param kernel An OpenCL kernel.
   * @param args The arguments to mote to the OpenCL kernel.
   * @tparam Args The types of <code>extra_args</code>.
   * @note Comes from:
   * simpleopencl.blogspot.com/2013/04/calling-kernels-with-large-number-of.html
   */
  template <typename... Args>
  inline void set_args(cl::Kernel& kernel, const Args&... args) {
    this->recursive_args(kernel, 0, args...);
  }
  template <typename... Args>
  inline void set_args(const Args&... args) {
    this->recursive_args(this->compiled_, 0, args...);
  }
  /**
   * return information on the kernel_source
   */
  inline kernel_cl_base::map_kernel_info kernel_info() {
    return kernel_cl_base::getInstance().kernel_info;
  }

  /**
   *
   */
  inline kernel_cl_base::map_kernel kernels() {
    return kernel_cl_base::getInstance().kernels;
  }

  inline kernel_cl_base::map_base_opts base_options() {
    return kernel_cl_base::getInstance().base_opts;
  }
};
}
}

 #endif
 #endif

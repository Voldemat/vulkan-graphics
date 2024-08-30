#include "./shaders.hpp"
#include <vector>

const std::vector<char> vertShaderCode = std::vector<char>(&data_start_shader_vert_spv,
                                       &data_end_shader_vert_spv);
const std::vector<char> fragShaderCode(&data_start_shader_frag_spv,
                                       &data_end_shader_frag_spv);

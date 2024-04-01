#include "DNIDynamicModelLoader.h"

#include <iostream>
#include <sstream>

namespace dni {

        void DNIDynamicModelLoader::Load()
        {
                // Parse input nodes infos in model
                input_nodes_num_ = session_->GetInputCount();
                std::cout << "Number of inputs = " << input_nodes_num_ << std::endl;

                input_node_names_.resize(input_nodes_num_);
                input_node_dims_.resize(input_nodes_num_);
                inputs_dynamic_status_.resize(input_nodes_num_, false);
                inputs_dynamic_index_.resize(input_nodes_num_, -1);
                inputs_size_in_1D_.resize(input_nodes_num_, 0);

                // Parse output nodes infos in model
                output_nodes_num_ = session_->GetOutputCount();
                std::cout << "\n\nNumber of outputs = " << output_nodes_num_ << std::endl;
                output_node_names_.resize(output_nodes_num_);

                Ort::AllocatorWithDefaultOptions allocator;
                std::size_t size_in_1D =
                    0;   // for node contain only one input, and the input is dynamic

                // iterate over all input nodes
                for (size_t i = 0; i < input_nodes_num_; i++)
                {
                        // Get input node names
                        char* input_name;
                        Ort::AllocatedStringPtr ptr =
                            session_->GetInputNameAllocated(i, allocator);
                        input_name = ptr.release();   // ptr.get();
                        input_node_names_[i] = input_name;
                        std::cout << "Input " << i << " : name = " << input_node_names_[i]
                                  << std::endl;

                        // Get input node types
                        Ort::TypeInfo typeinfo = session_->GetInputTypeInfo(i);
                        Ort::ConstTensorTypeAndShapeInfo tensor_info =
                            typeinfo.GetTensorTypeAndShapeInfo();

                        ONNXTensorElementDataType type;
                        type = tensor_info.GetElementType();
                        std::cout << "Input " << i << " : type = " << type << std::endl;
                        // assert type ???

                        // Get input shapes/dims/tensor_size
                        size_t num_dims;
                        num_dims = tensor_info.GetDimensionsCount();
                        std::cout << "Input " << i << " : num_dims = " << num_dims
                                  << std::endl;
                        input_node_dims_[i].resize(num_dims);
                        input_node_dims_[i] = tensor_info.GetShape();

                        for (size_t j = 0; j < num_dims; j++)
                        {
                                std::cout << "Input " << i << " : dim " << j << " = "
                                          << input_node_dims_[i][j] << std::endl;

                                if (input_node_dims_[i][j] == -1)   // dynamic dim
                                {
                                        if (inputs_dynamic_status_[i])
                                        {
                                                std::cout
                                                    << "Error, In input node [" << i
                                                    << "], dim ["
                                                    << inputs_dynamic_index_[i]
                                                    << "] is already dynamic, can not "
                                                       "contain more than one dynamic dim"
                                                    << std::endl;
                                                exit(1);
                                        }

                                        inputs_dynamic_status_[i] = true;
                                        inputs_dynamic_index_[i] = j;

                                        std::cout << "In input node [" << i << "], dim ["
                                                  << inputs_dynamic_index_[i]
                                                  << "] is dynamic." << std::endl;
                                }
                                else
                                {
                                        if (size_in_1D == 0)
                                        {
                                                size_in_1D = input_node_dims_[i][j];
                                        }
                                        else
                                        {
                                                size_in_1D *= input_node_dims_[i][j];
                                        }
                                }
                        }

                        inputs_size_in_1D_[i] = size_in_1D;

                        std::cout << "In input node [" << i << "], size_in_1D is "
                                  << inputs_size_in_1D_[i] << std::endl;

                        std::cout << std::endl;
                }

                // iterate over all output nodes
                for (size_t i = 0; i < output_nodes_num_; i++)
                {
                        // Get output node names
                        char* output_name;
                        Ort::AllocatedStringPtr ptr =
                            session_->GetOutputNameAllocated(i, allocator);
                        output_name = ptr.release();   // ptr.get();
                        output_node_names_[i] = output_name;
                        std::cout << "Output " << i
                                  << " : name = " << output_node_names_[i] << std::endl;

                        std::cout << std::endl;
                }
        }

        std::vector<Ort::Value> DNIDynamicModelLoader::Inference(
            std::vector<std::vector<float>*>& input_data)
        {
                std::vector<Ort::Value> input_tensors;
                Ort::MemoryInfo memory_info =
                    Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

                for (size_t i = 0; i < input_data.size(); i++)
                {
                        // only in this step, can determin the dynamic dim of each input
                        // node
                        if (inputs_dynamic_status_[i])
                        {
                                // if inputs_size_in_1D_[i] == 0, it is one dim input, and
                                // is dynamic
                                int fix_dim =
                                    ((inputs_size_in_1D_[i] == 0)
                                         ? (input_data[i]->size())
                                         : (input_data[i]->size() /
                                            inputs_size_in_1D_[i]));
                                std::cout << "In input node [" << i << "], fix_dim is "
                                          << fix_dim << std::endl;

                                input_node_dims_[i][inputs_dynamic_index_[i]] = fix_dim;
                        }

                        // debug
                        for (size_t m = 0; m < input_node_dims_[i].size(); m++)
                        {
                                std::cout << input_node_dims_[i][m] << "\t";
                        }
                        std::cout << std::endl;

                        input_tensors.push_back(Ort::Value::CreateTensor(
                            memory_info, input_data[i]->data(), input_data[i]->size(),
                            input_node_dims_[i].data(), input_node_dims_[i].size()));
                }

                try
                {
                        return std::move(session_->Run(
                            Ort::RunOptions{nullptr}, input_node_names_.data(),
                            input_tensors.data(), input_tensors.size(),
                            output_node_names_.data(), output_node_names_.size()));
                }
                catch (const Ort::Exception& exception)
                {
                        std::ostringstream oss;
                        oss << "ERROR running model inference: model is [" << modelPath_
                            << "], exception: " << exception.what();
                        std::cout << oss.str() << std::endl;

                        exit(1);
                }
        }

}   // namespace dni
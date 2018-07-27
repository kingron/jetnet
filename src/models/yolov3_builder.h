#ifndef YOLOV3_BUILDER_H
#define YOLOV3_BUILDER_H

#include "model_builder.h"
#include "darknet_weights_loader.h"
#include "conv2d_batch_leaky.h"
#include "upsample_plugin.h"
#include "fp16.h"
#include <NvInfer.h>
#include <memory>
#include <string>

namespace jetnet
{

class Yolov3Builder : public ModelBuilder
{
public:
    struct OutputSpec
    {
        std::string blob_name;
        int num_anchors;
        int num_classes;
        int num_coords;
    };

    /*
     *  Determine network properties using the constructor
     *  input_blob_name:    name of the input tensor. The same name must be used in the runner
     *                      to identify the input tensor
     *  weightsfile:        darknet weights file
     *  input_dimenstions:  network input dimensions (num channels, height, width)
     *  output_large:       specifications of output blob for detecting large sized objects
     *  output_mid:         specifications of output blob for detecting medium sized objects
     *  output_small:       specifications of output blob for detecting small sized objects
     */
    Yolov3Builder(std::string input_blob_name,
                  std::string weightsfile,
                  nvinfer1::DimsCHW input_dimenstions,
                  OutputSpec output_large,
                  OutputSpec output_mid,
                  OutputSpec output_small);

    nvinfer1::INetworkDefinition* parse(nvinfer1::DataType dt) override;

private:
    /*
     *  Network configuration
     */
    std::string m_input_blob_name;
    std::string m_weightsfile;
    nvinfer1::DimsCHW m_input_dimensions;
    OutputSpec m_output_large;
    OutputSpec m_output_mid;
    OutputSpec m_output_small;

    /*
     *  layer states (weights, layers and plugins)
     */
    std::unique_ptr<DarknetWeightsLoader> m_weights;
    std::unique_ptr<UpsamplePlugin> m_upsample_plugin0;
    std::unique_ptr<UpsamplePlugin> m_upsample_plugin1;
    Conv2dBatchLeaky m_convs[72];

};

}

#endif /* YOLOV3_BUILDER_H */

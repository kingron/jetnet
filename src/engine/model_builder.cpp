#include "model_builder.h"
#include "file_io.h"

using namespace jetnet;
using namespace nvinfer1;

bool ModelBuilder::init(std::shared_ptr<Logger> logger)
{
    m_logger = logger;
    m_builder = createInferBuilder(*m_logger);
    if (!m_builder)
        return false;

    m_network = m_builder->createNetwork();
    return m_network != nullptr;
}

bool ModelBuilder::platform_supports_fp16()
{
    return m_builder->platformHasFastFp16();
}

bool ModelBuilder::platform_supports_int8()
{
    return m_builder->platformHasFastInt8();
}

void ModelBuilder::platform_set_fp16_mode()
{
#if (NV_TENSORRT_MAJOR <= 3)
    m_builder->setHalf2Mode(true);
#else
    m_builder->setFp16Mode(true);
#endif
}

void ModelBuilder::platform_use_dla(int device)
{

    (void)device;
    m_logger->log(ILogger::Severity::kERROR, "Building for DLA is not supported in this version of TensorRT"
                  ", ignoring request");

}

void ModelBuilder::platform_set_int8_mode(IInt8Calibrator* calibrator)
{
    m_builder->setInt8Mode(true);
    m_builder->setInt8Calibrator(calibrator);
}

void ModelBuilder::enable_type_strictness()
{

    m_logger->log(ILogger::Severity::kERROR, "Type strictness is not supported in this version of TensorRT"
                  ", ignoring request");

}

void ModelBuilder::set_layer_precision(std::vector<std::string> layers,
                                       nvinfer1::DataType type, bool invert)
{

    (void)layers;
    (void)type;
    (void)invert;
    m_logger->log(ILogger::Severity::kERROR, "Setting layer-wise precision is not supported in this version of TensorRT"
                  ", ignoring request");

}

ICudaEngine* ModelBuilder::build(int maxBatchSize)
{
    m_builder->setMaxBatchSize(maxBatchSize);
    m_builder->setMaxWorkspaceSize(1 << 20);
    m_cuda_engine = m_builder->buildCudaEngine(*m_network);
    m_network->destroy();

    return m_cuda_engine;
}

IHostMemory* ModelBuilder::serialize()
{
    return m_cuda_engine->serialize();
}

IHostMemory* ModelBuilder::serialize(std::string filename)
{
    IHostMemory* stream = serialize();
    if (stream == nullptr)
        return nullptr;

    if (!write_binary_file(stream->data(), stream->size(), filename))
        return nullptr;

    m_cuda_engine->destroy();
    m_builder->destroy();

    return stream;
}

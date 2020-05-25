/*
 #
 #  File        : GmicSharpNative.cpp
 #                ( C++ source file )
 #
 #  Description : The native support library for gmic-sharp.
 #
 #  Copyright   : Nicholas Hayes
 #                ( https://github.com/0xC0000054 )
 #
 #  Licenses    : This file is 'dual-licensed', you have to choose one
 #                of the two licenses below to apply.
 #
 #                CeCILL-C
 #                The CeCILL-C license is close to the GNU LGPL.
 #                ( http://cecill.info/licences/Licence_CeCILL-C_V1-en.html )
 #
 #            or  CeCILL v2.1
 #                The CeCILL license is compatible with the GNU GPL.
 #                ( http://cecill.info/licences/Licence_CeCILL_V2.1-en.html )
 #
 #  This software is governed either by the CeCILL or the CeCILL-C license
 #  under French law and abiding by the rules of distribution of free software.
 #  You can  use, modify and or redistribute the software under the terms of
 #  the CeCILL or CeCILL-C licenses as circulated by CEA, CNRS and INRIA
 #  at the following URL: "http://cecill.info".
 #
 #  As a counterpart to the access to the source code and  rights to copy,
 #  modify and redistribute granted by the license, users are provided only
 #  with a limited warranty  and the software's author,  the holder of the
 #  economic rights,  and the successive licensors  have only  limited
 #  liability.
 #
 #  In this respect, the user's attention is drawn to the risks associated
 #  with loading,  using,  modifying and/or developing or reproducing the
 #  software by the user in light of its specific status of free software,
 #  that may mean  that it is complicated to manipulate,  and  that  also
 #  therefore means  that it is reserved for developers  and  experienced
 #  professionals having in-depth computer knowledge. Users are therefore
 #  encouraged to load and test the software's suitability as regards their
 #  requirements in conditions enabling the security of their systems and/or
 #  data to be ensured and,  more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL and CeCILL-C licenses and that you accept its terms.
 #
*/

#include "GmicSharpNative.h"
#include "CImg.h"
#include "gmic.h"
#include <string>
#include <stdexcept>
#include <memory>

class GmicImageList
{
public:
    gmic_list<float> images;
    gmic_list<char> names;

    GmicImageList() :
        images(), names()
    {
    }

    void clear()
    {
        images.clear();
        names.clear();
    }

    void ensure_capacity(unsigned int newCapacity)
    {
        if (images._allocated_width < newCapacity)
        {
            gmic_list<float> temp(newCapacity);

            if (images.size() > 0)
            {
                images.move_to(temp);
            }
            images.swap(temp);
        }
        else
        {
            images._width = newCapacity;
        }

        if (names._allocated_width < newCapacity)
        {
            gmic_list<char> temp(newCapacity);

            if (names.size() > 0)
            {
                names.move_to(temp);
            }
            names.swap(temp);
        }
        else
        {
            names._width = newCapacity;
        }
    }

    unsigned int get_next_available_index()
    {
        for (unsigned int i = 0; i < images.size(); i++)
        {
            if (images[i].is_empty())
            {
                return i;
            }
        }

        uint32_t nextIndex = images.size();

        if (nextIndex < std::numeric_limits<unsigned int>::max())
        {
            return nextIndex;
        }
        else
        {
            throw std::overflow_error("Adding more items to the list would cause an integer overflow.");
        }
    }

    unsigned int size() const
    {
        return images.size();
    }

private:
};

namespace
{
    unsigned int GetChannelCount(ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::Gray8:
            return 1;
        case ImageFormat::Bgr888:
        case ImageFormat::Rgb888:
        case ImageFormat::Bgr888x:
        case ImageFormat::Rgb888x:
            return 3;
        case ImageFormat::Bgra8888:
        case ImageFormat::Rgba8888:
            return 4;
        default:
            return 0;
        }
    }

    inline unsigned char ClampToUint8(float value)
    {
        return static_cast<uint8_t>(value > 255.0f ? 255.0f : value < 0.0f ? 0.0f : value);
    }

    void CopyFromGray(
        gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        const void* data)
    {
        float* dst = image->data(0, 0, 0, 0);

        for (unsigned int y = 0; y < height; y++)
        {
            const uint8_t* src = static_cast<const uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                *dst++ = static_cast<float>(*src++);
            }
        }
    }

    void CopyFromRgbOrBgr(
        gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        const void* data,
        ImageFormat format)
    {
        float* dstR = image->data(0, 0, 0, 0);
        float* dstG = image->data(0, 0, 0, 1);
        float* dstB = image->data(0, 0, 0, 2);

        for (unsigned int y = 0; y < height; y++)
        {
            const uint8_t* src = static_cast<const uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgr888:
                    *dstB++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstR++ = static_cast<float>(src[2]);
                    break;
                case ImageFormat::Rgb888:
                    *dstR++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstB++ = static_cast<float>(src[2]);
                    break;
                }

                src += 3;
            }
        }
    }

    void CopyFromRgbaOrBgra(
        gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        const void* data,
        ImageFormat format)
    {
        float* dstR = image->data(0, 0, 0, 0);
        float* dstG = image->data(0, 0, 0, 1);
        float* dstB = image->data(0, 0, 0, 2);
        float* dstA = image->data(0, 0, 0, 3);

        for (unsigned int y = 0; y < height; y++)
        {
            const uint8_t* src = static_cast<const uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgra8888:
                    *dstB++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstR++ = static_cast<float>(src[2]);
                    *dstA++ = static_cast<float>(src[3]);
                    break;
                case ImageFormat::Rgba8888:
                    *dstR++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstB++ = static_cast<float>(src[2]);
                    *dstA++ = static_cast<float>(src[3]);
                    break;
                }

                src += 4;
            }
        }
    }

    void CopyFromRgbxOrBgrx(
        gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        const void* data,
        ImageFormat format)
    {
        float* dstR = image->data(0, 0, 0, 0);
        float* dstG = image->data(0, 0, 0, 1);
        float* dstB = image->data(0, 0, 0, 2);

        for (unsigned int y = 0; y < height; y++)
        {
            const uint8_t* src = static_cast<const uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgr888x:
                    *dstB++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstR++ = static_cast<float>(src[2]);
                    break;
                case ImageFormat::Rgb888x:
                    *dstR++ = static_cast<float>(src[0]);
                    *dstG++ = static_cast<float>(src[1]);
                    *dstB++ = static_cast<float>(src[2]);
                    break;
                }

                src += 4;
            }
        }
    }

    void CopyToGray(
        const gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        void* data,
        ImageFormat format)
    {
        const float* src = image->data(0, 0, 0, 0);

        for (unsigned int y = 0; y < height; y++)
        {
            uint8_t* dst = static_cast<uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                uint8_t gray = ClampToUint8(*src++);

                switch (format)
                {
                case ImageFormat::Gray8:
                    *dst++ = gray;
                    break;
                case ImageFormat::Bgr888:
                case ImageFormat::Rgb888:
                    dst[0] = dst[1] = dst[2] = gray;
                    dst += 3;
                    break;
                case ImageFormat::Bgr888x:
                case ImageFormat::Rgb888x:
                    dst[0] = dst[1] = dst[2] = gray;
                    dst += 4;
                    break;
                case ImageFormat::Bgra8888:
                case ImageFormat::Rgba8888:
                    dst[0] = dst[1] = dst[2] = gray;
                    dst[3] = 255;
                    dst += 4;
                    break;
                }
            }
        }
    }

    void CopyToGrayWithAlpha(
        const gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        void* data)
    {
        const float* srcGray = image->data(0, 0, 0, 0);
        const float* srcA = image->data(0, 0, 0, 1);

        for (unsigned int y = 0; y < height; y++)
        {
            uint8_t* dst = static_cast<uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                dst[0] = dst[1] = dst[2] = ClampToUint8(*srcGray++);
                dst[3] = ClampToUint8(*srcA++);

                dst += 4;
            }
        }
    }

    void CopyToRgbOrBgr(
        const gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        void* data,
        ImageFormat format)
    {
        const float* srcR = image->data(0, 0, 0, 0);
        const float* srcG = image->data(0, 0, 0, 1);
        const float* srcB = image->data(0, 0, 0, 2);

        for (unsigned int y = 0; y < height; y++)
        {
            uint8_t* dst = static_cast<uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgr888:
                    dst[0] = ClampToUint8(*srcB++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcR++);
                    break;
                case ImageFormat::Rgb888:
                    dst[0] = ClampToUint8(*srcR++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcB++);
                    break;
                }

                dst += 3;
            }
        }
    }

    void CopyToRgbaOrBgra(
        const gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        void* data,
        ImageFormat format)
    {
        const float* srcR = image->data(0, 0, 0, 0);
        const float* srcG = image->data(0, 0, 0, 1);
        const float* srcB = image->data(0, 0, 0, 2);
        const float* srcA = image->data(0, 0, 0, 3);

        for (unsigned int y = 0; y < height; y++)
        {
            uint8_t* dst = static_cast<uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgra8888:
                    dst[0] = ClampToUint8(*srcB++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcR++);
                    dst[3] = ClampToUint8(*srcA++);
                    break;
                case ImageFormat::Rgba8888:
                    dst[0] = ClampToUint8(*srcR++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcB++);
                    dst[3] = ClampToUint8(*srcA++);
                    break;
                }

                dst += 4;
            }
        }
    }

    void CopyToRgbxOrBgrx(
        const gmic_image<float>* image,
        unsigned int width,
        unsigned int height,
        unsigned int stride,
        void* data,
        ImageFormat format)
    {
        const float* srcR = image->data(0, 0, 0, 0);
        const float* srcG = image->data(0, 0, 0, 1);
        const float* srcB = image->data(0, 0, 0, 2);

        for (unsigned int y = 0; y < height; y++)
        {
            uint8_t* dst = static_cast<uint8_t*>(data) + (static_cast<size_t>(y) * stride);

            for (unsigned int x = 0; x < width; x++)
            {
                switch (format)
                {
                case ImageFormat::Bgra8888:
                    dst[0] = ClampToUint8(*srcB++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcR++);
                    dst[3] = 255;
                    break;
                case ImageFormat::Rgba8888:
                    dst[0] = ClampToUint8(*srcR++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcB++);
                    dst[3] = 255;
                    break;
                case ImageFormat::Bgr888x:
                    dst[0] = ClampToUint8(*srcB++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcR++);
                    break;
                case ImageFormat::Rgb888x:
                    dst[0] = ClampToUint8(*srcR++);
                    dst[1] = ClampToUint8(*srcG++);
                    dst[2] = ClampToUint8(*srcB++);
                    break;
                }

                dst += 4;
            }
        }
    }

    bool CheckGmicUpdateFileSignature(std::FILE* file)
    {
        bool result = false;

        char sig[8] = {};
        constexpr size_t gmicSigSize = 6;

        if (std::fread(sig, 1, gmicSigSize, file) == gmicSigSize)
        {
            result = memcmp(sig, "#@gmic", gmicSigSize) == 0;
        }

        std::rewind(file);

        return result;
    }

    void AddCommandsFromResources(
        const char* const customResourcePath,
        gmic& gmicInstance)
    {
        using namespace cimg_library;

        char updateFilePath[1024] = {};

        cimg_snprintf(updateFilePath, sizeof(updateFilePath), "%supdate%u.gmic", gmic::path_rc(customResourcePath), gmic_version);

        std::FILE* updateFile = std::fopen(updateFilePath, "rb");

        if (updateFile)
        {
            try
            {
                if (CheckGmicUpdateFileSignature(updateFile))
                {
                    gmicInstance.add_commands(updateFile);
                }
            }
            catch (...)
            {
                // Ignore any errors from add_commands
            }

            std::fclose(updateFile);
        }

        std::FILE* userFile = std::fopen(gmic::path_user(customResourcePath), "rb");

        if (userFile)
        {
            try
            {
                gmicInstance.add_commands(userFile);
            }
            catch (...)
            {
                // Ignore any errors from add_commands
            }

            std::fclose(userFile);
        }
    }
}

GmicImageList* GSN_API GmicImageListCreate()
{
    try
    {
        return new GmicImageList();
    }
    catch (...)
    {
        return nullptr;
    }
}

void GSN_API GmicImageListDestroy(GmicImageList* imageList)
{
    if (imageList)
    {
        delete imageList;
    }
}

void GSN_API GmicImageListClear(GmicImageList* list)
{
    if (list)
    {
        list->clear();
    }
}

unsigned int GSN_API GmicImageListGetCount(GmicImageList* list)
{
    unsigned int count = 0;

    if (list)
    {
        count = list->size();
    }

    return count;
}

GmicStatus GSN_API GmicImageListGetImageInfo(
    GmicImageList* list,
    unsigned int index,
    GmicImageListItemInfo* info)
{
    if (!list || !info)
    {
        return GmicStatus::InvalidParameter;
    }

    if (index >= list->size())
    {
        return GmicStatus::ImageListIndexOutOfRange;
    }

    gmic_image<float>* image = list->images.data(index);

    info->width = static_cast<unsigned int>(image->width());
    info->height = static_cast<unsigned int>(image->height());

    const int spectrum = image->spectrum();

    switch (spectrum)
    {
    case 1: // Gray
        info->format = ImageFormat::Gray8;
        break;
    case 3: // RGB
        info->format = ImageFormat::Rgb888;
        break;
    case 2: // Gray + Alpha
    case 4: // RGBA
        info->format = ImageFormat::Rgba8888;
        break;
    }

    return GmicStatus::Ok;
}

GmicStatus GSN_API GmicImageListAdd(
    GmicImageList* list,
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    const void* data,
    ImageFormat format,
    const char* name)
{
    if (!list || !width || !height || !stride || !data)
    {
        return GmicStatus::InvalidParameter;
    }

    try
    {
        unsigned int index = list->get_next_available_index();

        if (index == list->size())
        {
            list->ensure_capacity(index + 1);
        }

        const unsigned int channelCount = GetChannelCount(format);

        if (!channelCount)
        {
            return GmicStatus::UnknownImageFormat;
        }

        gmic_image<float>* image = list->images.data(index);

        if (name)
        {
            list->names[index] = name;
        }
        else
        {
            char imageNameBuffer[32];

            std::sprintf(imageNameBuffer, "image %llu", static_cast<uint64_t>(index) + 1);

            list->names[index] = imageNameBuffer;
        }

        image->assign(width, height, 1, channelCount);

        switch (format)
        {
        case ImageFormat::Gray8:
            CopyFromGray(image, width, height, stride, data);
            break;
        case ImageFormat::Bgr888:
        case ImageFormat::Rgb888:
            CopyFromRgbOrBgr(image, width, height, stride, data, format);
            break;
        case ImageFormat::Bgr888x:
        case ImageFormat::Rgb888x:
            CopyFromRgbxOrBgrx(image, width, height, stride, data, format);
            break;
        case ImageFormat::Bgra8888:
        case ImageFormat::Rgba8888:
            CopyFromRgbaOrBgra(image, width, height, stride, data, format);
            break;
        default:
            return GmicStatus::UnknownImageFormat;
        }
    }
    catch (const std::bad_alloc&)
    {
        return GmicStatus::OutOfMemory;
    }
    catch (const std::overflow_error&)
    {
        return GmicStatus::OutOfMemory;
    }
    catch (...)
    {
        return GmicStatus::UnknownError;
    }

    return GmicStatus::Ok;
}

GmicStatus GSN_API GmicImageListCopyToOutput(GmicImageList* list, unsigned int index, unsigned int width, unsigned int height, unsigned int stride, void* data, ImageFormat format)
{
    if (!list || !width || !height || !stride || !data)
    {
        return GmicStatus::InvalidParameter;
    }

    if (index >= list->size())
    {
        return GmicStatus::ImageListIndexOutOfRange;
    }

    const gmic_image<float>* image = list->images.data(index);

    const int spectrum = image->spectrum();

    if (spectrum == 3)
    {
        switch (format)
        {
        case ImageFormat::Bgr888:
        case ImageFormat::Rgb888:
            CopyToRgbOrBgr(image, width, height, stride, data, format);
            break;
        case ImageFormat::Bgr888x:
        case ImageFormat::Rgb888x:
        case ImageFormat::Bgra8888:
        case ImageFormat::Rgba8888:
            CopyToRgbxOrBgrx(image, width, height, stride, data, format);
            break;
        default:
            return GmicStatus::InvalidParameter;
        }
    }
    else if (spectrum == 4)
    {
        if (format != ImageFormat::Bgra8888 && format != ImageFormat::Rgba8888)
        {
            return GmicStatus::InvalidParameter;
        }

        CopyToRgbaOrBgra(image, width, height, stride, data, format);
    }
    else if (spectrum == 2)
    {
        if (format != ImageFormat::Bgra8888 && format != ImageFormat::Rgba8888)
        {
            return GmicStatus::InvalidParameter;
        }

        CopyToGrayWithAlpha(image, width, height, stride, data);
    }
    else if (spectrum == 1)
    {
        switch (format)
        {
        case ImageFormat::Gray8:
        case ImageFormat::Bgr888:
        case ImageFormat::Bgr888x:
        case ImageFormat::Bgra8888:
        case ImageFormat::Rgb888:
        case ImageFormat::Rgb888x:
        case ImageFormat::Rgba8888:
            CopyToGray(image, width, height, stride, data, format);
            break;
        default:
            return GmicStatus::InvalidParameter;
        }

    }
    else
    {
        return GmicStatus::GmicUnsupportedChannelCount;
    }

    return GmicStatus::Ok;
}

GmicStatus GSN_API RunGmic(GmicImageList* list, GmicOptions* options, GmicErrorInfo* errorInfo)
{
    if (!list || !options)
    {
        return GmicStatus::InvalidParameter;
    }

    try
    {
        if (!gmic::init_rc(options->customResourcePath))
        {
            return GmicStatus::GmicResourcePathInitFailed;
        }

        gmic gmicInstance;

        AddCommandsFromResources(options->customResourcePath, gmicInstance);

        gmicInstance.set_variable("_host", options->hostName ? options->hostName : "gmic-sharp", 0);

        gmicInstance.run(options->commandLine, list->images, list->names, options->progress, options->abort);
    }
    catch (const std::bad_alloc&)
    {
        return GmicStatus::OutOfMemory;
    }
    catch (const gmic_exception& e)
    {
        if (errorInfo)
        {
            size_t commandLength = std::min(255U, e._command._width);

            if (commandLength)
            {
                std::memcpy(errorInfo->commandName, e._command.data(), commandLength);
                errorInfo->commandName[commandLength] = 0;
            }

            size_t messageLength = std::min(255U, e._message._width);

            if (messageLength)
            {
                std::memcpy(errorInfo->errorMessage, e._message.data(), messageLength);
                errorInfo->errorMessage[messageLength] = 0;
            }
        }

        return GmicStatus::GmicError;
    }
    catch (...)
    {
        return GmicStatus::UnknownError;
    }

    return GmicStatus::Ok;
}




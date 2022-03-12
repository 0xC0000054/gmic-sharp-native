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
#include "version.h"
#include "CImg.h"

// Remove the gmic_build define before including gmic.h
// it causes linker errors when used outside of gmic.cpp
#ifdef gmic_build
#undef gmic_build
#endif // gmic_build

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

        unsigned int nextIndex = images.size();

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

        std::FILE* updateFile = cimg::std_fopen(updateFilePath, "rb");

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

            cimg::fclose(updateFile);
        }

        std::FILE* userFile = cimg::std_fopen(gmic::path_user(customResourcePath), "rb");

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

            cimg::fclose(userFile);
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

unsigned int GSN_API GmicImageListGetCount(GmicImageList* list)
{
    unsigned int count = 0;

    if (list)
    {
        count = list->size();
    }

    return count;
}

GmicStatus GSN_API GmicImageListGetImageData(
    GmicImageList* list,
    unsigned int index,
    GmicImageListImageData* data)
{
    if (!list || !data)
    {
        return GmicStatus::InvalidParameter;
    }

    if (index >= list->size())
    {
        return GmicStatus::ImageListIndexOutOfRange;
    }

    if (data->version != 1)
    {
        return GmicStatus::UnsupportedStructureVersion;
    }

    gmic_image<float>* image = list->images.data(index);

    data->width = static_cast<unsigned int>(image->width());
    data->height = static_cast<unsigned int>(image->height());

    const int spectrum = image->spectrum();

    if (spectrum == 3) // RGB
    {
        data->pixels.red = image->data(0, 0, 0, 0);
        data->pixels.green = image->data(0, 0, 0, 1);
        data->pixels.blue = image->data(0, 0, 0, 2);
        data->format = ImageFormat::Rgb;
    }
    else if (spectrum == 4) // RGB + Alpha
    {
        data->pixels.red = image->data(0, 0, 0, 0);
        data->pixels.green = image->data(0, 0, 0, 1);
        data->pixels.blue = image->data(0, 0, 0, 2);
        data->pixels.alpha = image->data(0, 0, 0, 3);
        data->format = ImageFormat::RgbAlpha;
    }
    else if (spectrum == 2) // Gray + Alpha
    {
        data->pixels.gray = image->data(0, 0, 0, 0);
        data->pixels.alpha = image->data(0, 0, 0, 1);
        data->format = ImageFormat::GrayAlpha;
    }
    else if (spectrum == 1) // Gray
    {
        data->pixels.gray = image->data(0, 0, 0, 0);
        data->format = ImageFormat::Gray;
    }
    else
    {
        return GmicStatus::GmicUnsupportedChannelCount;
    }

    if (index < list->names.size())
    {
        const cimg_library::CImg<char>* name = list->names.data(index);

        if (name->width() > 0)
        {
            data->name = name->data();
            int nameLength = name->width();

            // Subtract the terminator from the name length.
            if (data->name[nameLength - 1] == '\0')
            {
                nameLength -= 1;
            }

            data->nameLength = nameLength;
        }
        else
        {
            data->name = nullptr;
            data->nameLength = 0;
        }
    }
    else
    {
        data->name = nullptr;
        data->nameLength = 0;
    }

    return GmicStatus::Ok;
}

GmicStatus GSN_API GmicImageListAdd(
    GmicImageList* list,
    unsigned int width,
    unsigned int height,
    ImageFormat format,
    const char* name,
    GmicImageListPixelData* data)
{
    if (!list || !width || !height || !data)
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

        unsigned int channelCount;

        switch (format)
        {
        case ImageFormat::Gray:
            channelCount = 1;
            break;
        case ImageFormat::GrayAlpha:
            channelCount = 2;
            break;
        case ImageFormat::Rgb:
            channelCount = 3;
            break;
        case ImageFormat::RgbAlpha:
            channelCount = 4;
            break;
        default:
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

            std::sprintf(imageNameBuffer, "image " cimg_fuint64, static_cast<uint64_t>(index) + 1);

            list->names[index] = imageNameBuffer;
        }

        image->assign(width, height, 1, channelCount);

        switch (format)
        {
        case ImageFormat::Gray:
            data->gray = image->data(0, 0, 0, 0);
            break;
        case ImageFormat::GrayAlpha:
            data->gray = image->data(0, 0, 0, 0);
            data->alpha = image->data(0, 0, 0, 1);
            break;
        case ImageFormat::Rgb:
            data->red = image->data(0, 0, 0, 0);
            data->green = image->data(0, 0, 0, 1);
            data->blue = image->data(0, 0, 0, 2);
            break;
        case ImageFormat::RgbAlpha:
            data->red = image->data(0, 0, 0, 0);
            data->green = image->data(0, 0, 0, 1);
            data->blue = image->data(0, 0, 0, 2);
            data->alpha = image->data(0, 0, 0, 3);
            break;
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




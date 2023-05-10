#pragma once

#ifndef CXCLOUD_MODEL_HISTORY_TOOL_H
#define CXCLOUD_MODEL_HISTORY_TOOL_H

#include <QtCore/QString>

#include "../interface.hpp"

namespace cxcloud {

CXCLOUD_API void PushModelHistory(const QString& json_string);

CXCLOUD_API QString LoadModelHistory(int page, int size);

}  // namespace cxcloud

#endif  // #define CXCLOUD_MODEL_HISTORY_TOOL_H

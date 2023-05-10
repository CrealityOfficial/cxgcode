#include "history_tool.h"

#include <map>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>

// model history fromat in settings example:
// 
// "model_history":        // group
//   "vector": "{          // [string]           vector json
//     "size": 10,         // [number]           vector size
//     "0": "1919810..."   // [string] model id, oldest
//     ...                 
//     "9": "114514...."   // [string] model id, newest
//   }"
//   "1919810...": "{...}" // [string] model id -> model json
//   "114514....": "{...}" // [string] model id -> model json
// 
// model history fromat in qml example:
// 
// {
//   "cur_page": 0 // [number]
//   "max_page": 2 // [number]
//   "size": 10,   // [number] vector size
//   "0": {...},   // [object] oldest
//   ...           
//   "9": {...}    // [object] newest
// }
// 
// history model json fromat example:
// 
// {
//   "model_id"       : "1919810.. .", // [string]
//   "model_image"    : "https://...", // [string]
//   "model_name"     : "xxxxxxxxxxx", // [string]
//   "model_count"    : 9,             // [number]
//   "model_time"     : 14514........, // [number]
//   "total_price"    : 1000         , // [number]
//   "author_nickname": "xxxxxxxxxxx", // [string]
//   "author_avatar"  : "https://..."  // [string]
//   "collected"      : false          // [boolean]
// }

namespace {

constexpr auto GROUP   { "model_history" };
constexpr auto VECTOR  { "vector"        };
constexpr auto SIZE    { "size"          };
constexpr auto CUR_PAGE{ "cur_page"      };
constexpr auto MAX_PAGE{ "max_page"      };
constexpr auto MODEL_ID{ "model_id"      };

static const std::map<QString, QJsonValue::Type> KEY_TYPE_MAP{
  { QStringLiteral("model_id")        , QJsonValue::Type::String },
  { QStringLiteral("model_image")     , QJsonValue::Type::String },
  { QStringLiteral("model_name")      , QJsonValue::Type::String },
  { QStringLiteral("model_count")     , QJsonValue::Type::Double },
  { QStringLiteral("model_time")      , QJsonValue::Type::Double },
  { QStringLiteral("total_price")     , QJsonValue::Type::Double },
  { QStringLiteral("author_nickname") , QJsonValue::Type::String },
  { QStringLiteral("author_avatar")   , QJsonValue::Type::String },
  { QStringLiteral("collected")       , QJsonValue::Type::Bool   },
};

} // namespace

namespace cxcloud {

void PushModelHistory(const QString& json_string) {
  if (json_string.isEmpty()) {
    return;
  }

  // json format check
  QJsonParseError error;
  auto document = QJsonDocument::fromJson(json_string.toUtf8(), &error);
  if (error.error != QJsonParseError::ParseError::NoError) {
    return;
  }

  // data format check
  auto model_json = document.object();
  if (model_json.empty()) { return; }
  for (const auto& pair : KEY_TYPE_MAP) {
    if (model_json.value(pair.first).type() != pair.second) {
      return;
    }
  }

  // start update settings
  QSettings settings;
  settings.beginGroup(GROUP);

  // read vector
  auto vector_string = settings.value(VECTOR, {}).value<QString>();
  document = QJsonDocument::fromJson(vector_string.toUtf8(), &error);
  const auto vector_vaild = error.error == QJsonParseError::ParseError::NoError;
  auto vector_json = vector_vaild ? document.object() : QJsonObject{};

  // update vector : resort if need
  auto resort = false;
  const auto vector_size = vector_json.value(SIZE).toInt(0);
  const auto model_id = model_json.value(MODEL_ID).toString();
  // don't use range for or iterator, we sort vector ourselves
  for (size_t index = 0; index < vector_size; index++) {
    const auto value = vector_json.value(QString::number(index));
    // vaild check
    if (value.isUndefined() || value.isNull() || !value.isString()) {
      continue;
    }

    // if new model is already in history, resort the rest of vector
    if (value.toString() == model_id) {
      resort = true;
      continue;
    }

    // only rest items of vector will come into
    if (resort) {
      vector_json[QString::number(index - 1)] = value;
    }
  }

  // update vector : push new model id
  const auto model_index = vector_size - (resort ? 1 : 0);
  vector_json[QString::number(model_index)] = model_id;
  vector_json[SIZE] = model_index + 1;
  document = QJsonDocument{ std::move(vector_json) };
  vector_string = document.toJson(QJsonDocument::JsonFormat::Compact);

  // write new settings data, both new vector and new model (model info maybe updated)
  settings.setValue(VECTOR, QVariant::fromValue(std::move(vector_string)));
  settings.setValue(model_id, QVariant::fromValue(json_string));
  settings.endGroup();
}

QString LoadModelHistory(int load_page, int load_size) {
  QSettings settings;
  settings.beginGroup(GROUP);

  const auto vector_string = settings.value(VECTOR, {}).value<QString>();

  QJsonParseError error;
  auto document = QJsonDocument::fromJson(vector_string.toUtf8(), &error);
  const auto vector_vaild = error.error == QJsonParseError::ParseError::NoError;
  const auto vector_json = vector_vaild ? document.object() : QJsonObject{};
  const auto vector_size = vector_json.value(SIZE).toInt(0);

  std::vector<QJsonObject> model_vector;

  // read data from settings and filter dirty data
  for (size_t index = 0; index < vector_size; index++) {
    const auto value = vector_json.value(QString::number(index));
    // vector data vaild check
    if (value.isUndefined() || value.isNull() || !value.isString()) {
      continue;
    }

    // model data vaild check
    const auto model_variant = settings.value(value.toString());
    if (model_variant.isNull() || !model_variant.isValid() ||
      model_variant.type() != QVariant::Type::String) {
      continue;
    }

    // json format vaild check
    const auto model_string = model_variant.value<QString>();
    document = QJsonDocument::fromJson(model_string.toUtf8(), &error);
    if (error.error != QJsonParseError::ParseError::NoError) {
      continue;
    }

    // model format vaild check
    bool vaild = true;
    const auto model_json = document.object();
    for (const auto& pair : KEY_TYPE_MAP) {
      if (model_json.value(pair.first).type() != pair.second) {
        vaild = false;
        break;
      }
    }
    if (!vaild) { continue; }

    model_vector.push_back(model_json);
  }

  settings.endGroup();

  // caculate size, cur_page, max_page
  const auto model_size = model_vector.size();
  const auto beg_index = load_page * load_size;
  const auto end_index = std::min<size_t>(beg_index + load_size, model_size);
  const int read_size = std::max<size_t>(end_index - beg_index, 0);
  const int max_page = std::max<size_t>(model_size / load_size + (model_size % load_size == 0 ? 0 : 1) - 1, 0);

  // the json to be send to qml
  QJsonObject json_root{
    { SIZE    , read_size },
    { CUR_PAGE, load_page },
    { MAX_PAGE, max_page  },
  };

  for (size_t index = 0; index < read_size; index++) {
    json_root[QString::number(index)] = std::move(model_vector[index + beg_index]);
  }

  document = QJsonDocument{ std::move(json_root) };
  return document.toJson(QJsonDocument::JsonFormat::Compact);
}

} // namespace cxcloud

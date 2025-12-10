// conversions.cc

#include "absl.h"
#include <node_api.h>

#define DECLARE_NODE_API_PROPERTY(name, func)                                  \
  {(name), NULL, (func), NULL, NULL, NULL, napi_default, NULL}

#define NAPI_CALL(env, the_call) NAPI_CALL_BASE(env, the_call, NULL)

#define NAPI_CALL_BASE(env, the_call, ret_val)                           \
  do {                                                                   \
    if ((the_call) != napi_ok) {                                         \
      GET_AND_THROW_LAST_ERROR((env));                                   \
      return (ret_val);                                                  \
    }                                                                    \
  } while (0)

#define GET_AND_THROW_LAST_ERROR(env)                                    \
  do {                                                                   \
    const napi_extended_error_info *error_info;                          \
    napi_get_last_error_info((env), &error_info);                        \
    bool is_pending;                                                     \
    napi_is_exception_pending((env), &is_pending);                       \
    /* If an exception is already pending, don't rethrow it */           \
    if (!is_pending) {                                                   \
      const char* error_message = error_info->error_message != NULL ?    \
        error_info->error_message :                                      \
        "empty error message";                                           \
      napi_throw_error((env), NULL, error_message);                      \
    }                                                                    \
  } while (0)


namespace ima::addons {

absl::StatusOr<std::string> NewStringFromUint8Array(napi_env env,
                                                    napi_value value) {
  napi_status status;
  bool is_typedarray;
  status = napi_is_typedarray(env, value, &is_typedarray);
  if (status != napi_ok || !is_typedarray) {
    return absl::InvalidArgumentError("value was not a TypedArray");
  }

  napi_typedarray_type type;
  size_t length;
  void* data;
  napi_value arraybuffer;
  size_t byte_offset;
  status = napi_get_typedarray_info(
      env, value, &type, &length, &data, &arraybuffer, &byte_offset);
  if (status != napi_ok) {
    return absl::InternalError("Failed to get typedarray info");
  }

  if (type != napi_uint8_array) {
    return absl::InvalidArgumentError("value was not a Uint8Array");
  }

  return std::string(static_cast<char*>(data), length);
}

napi_value NewUint8ArrayFromString(napi_env env, absl::string_view string) {
  napi_value arraybuffer;
  void* data = nullptr;
  NAPI_CALL(env, napi_create_arraybuffer(env, string.size(), &data, &arraybuffer));

  absl::c_copy(string, static_cast<char*>(data));

  napi_value uint8_array;
  NAPI_CALL(env, napi_create_typedarray(
      env, napi_uint8_array, string.size(), arraybuffer, 0, &uint8_array));

  return uint8_array;
}

}  // namespace ima::addons

// main.cc

napi_value Encrypt(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];

  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

  if (argc != 2) {
    napi_throw_type_error(env, nullptr, "Wrong number of args!");
    return nullptr;
  }

  // arg0
  napi_valuetype valuetype0;
  NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
  if (valuetype0 != napi_string) {
    napi_throw_type_error(env, nullptr, "Wrong type of keyset_json!");
    return nullptr;
  }

  size_t str_size;
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &str_size));
  std::string keyset_json(str_size, '\0');
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &keyset_json[0], str_size + 1, &str_size));
  keyset_json.resize(str_size); // Adjust string size to actual length


  // arg1
  absl::StatusOr<std::string> clear_text =
      ima::addons::NewStringFromUint8Array(env, argv[1]);
  if (!clear_text.ok()) {
    napi_throw_type_error(
        env,
        nullptr,
        clear_text.status()
            .ToString(absl::StatusToStringMode::kWithNoExtraData)
            .c_str());
    return nullptr;
  }

  absl::StatusOr<std::string> cipher_text =
      ima::addons::encrypt::Encrypt(keyset_json, *clear_text);
  if (!cipher_text.ok()) {
    napi_throw_type_error(
        env,
        nullptr,
        cipher_text.status()
            .ToString(absl::StatusToStringMode::kWithNoExtraData)
            .c_str());
    return nullptr;
  }

  return ima::addons::NewUint8ArrayFromString(env, *cipher_text);
}

napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
      DECLARE_NODE_API_PROPERTY("encrypt", Encrypt)};
  NAPI_CALL(env, napi_define_properties(env, exports, 1, properties));
  return exports;
}

NAPI_MODULE(encrypt, Init)

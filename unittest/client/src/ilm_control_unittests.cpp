#include <gtest/gtest.h>
#include "wayland-util.h"
#include "ilm_control.h"
#include "ilm_control_platform.h"
#include "client_api_fake.h"
#include "ilm_control_base_class.hpp"
#include "ivi-wm-client-protocol.h"

extern "C"{
    extern struct ilm_control_context ilm_context;
}
static constexpr uint8_t MAX_NUMBER = 5;

enum ilmControlStatus
{
    CREATE_LAYER    = 0,
    DESTROY_LAYER   = 1,
    CREATE_SURFACE  = 2,
    DESTROY_SURFACE = 3,
    NONE = 4
};

static ilmControlStatus g_ilmControlStatus = NONE;

class IlmControlTest : public ::testing::Test, public IlmControlInitBase
{
public:
    void SetUp()
    {
        ASSERT_EQ(initBaseModule(), true);
        init_ctx_list_content();
        CLIENT_API_FAKE_LIST(RESET_FAKE);
        g_ilmControlStatus = NONE;
    }

    void TearDown()
    {
        deinit_ctx_list_content();
    }

    void init_ctx_list_content()
    {
        custom_wl_list_init(&ilm_context.wl.list_surface);
        custom_wl_list_init(&ilm_context.wl.list_screen);
        custom_wl_list_init(&ilm_context.wl.list_layer);
        ilm_context.wl.controller = (struct ivi_wm*)&m_iviWmControllerFakePointer;

        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            // prepare the surfaces
            mp_ctxSurface[i] = (struct surface_context*)malloc(sizeof(struct surface_context));
            mp_ctxSurface[i]->id_surface = mp_ilmSurfaceIds[i];
            mp_ctxSurface[i]->ctx = &ilm_context.wl;
            mp_ctxSurface[i]->prop = mp_surfaceProps[i];
            custom_wl_list_insert(&ilm_context.wl.list_surface, &mp_ctxSurface[i]->link);
            //prepare the layers
            mp_ctxLayer[i] = (struct layer_context*)malloc(sizeof(struct layer_context));
            mp_ctxLayer[i]->id_layer = mp_ilmLayerIds[i];
            mp_ctxLayer[i]->ctx = &ilm_context.wl;
            mp_ctxLayer[i]->prop = mp_layerProps[i];
            custom_wl_list_insert(&ilm_context.wl.list_layer, &mp_ctxLayer[i]->link);
            custom_wl_array_init(&mp_ctxLayer[i]->render_order);
            // prepare the screens
            mp_ctxScreen[i] = (struct screen_context*)malloc(sizeof(struct screen_context));
            mp_ctxScreen[i]->id_screen = mp_ilmScreenIds[i];
            mp_ctxScreen[i]->ctx = &ilm_context.wl;
            mp_ctxScreen[i]->prop = mp_screenProps[i];
            custom_wl_list_insert(&ilm_context.wl.list_screen, &mp_ctxScreen[i]->link);
            custom_wl_array_init(&mp_ctxScreen[i]->render_order);
        }
    }

    void deinit_ctx_list_content()
    {
        {
            struct surface_context *l, *n;
            wl_list_for_each_safe(l, n, &ilm_context.wl.list_surface, link) 
            {
                custom_wl_list_remove(&l->link);
            }
        }
        {
            struct layer_context *l, *n;
            wl_list_for_each_safe(l, n, &ilm_context.wl.list_layer, link) 
            {
                custom_wl_list_remove(&l->link);
            }
        }
        {
            struct screen_context *l, *n;
            wl_list_for_each_safe(l, n, &ilm_context.wl.list_screen, link) 
            {
                custom_wl_list_remove(&l->link);
            }
        }
        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            if(mp_ctxSurface[i] != nullptr)
            {
                free(mp_ctxSurface[i]);
            }
            if(mp_ctxLayer[i] != nullptr)
            {
                free(mp_ctxLayer[i]);
            }
            if(mp_ctxScreen[i] != nullptr)
            {
                free(mp_ctxScreen[i]);
            }
        }
        ilm_context.wl.controller = nullptr;
        ilm_context.wl.notification = nullptr;
        ilm_context.initialized = false;
    }

    t_ilm_surface mp_ilmSurfaceIds[MAX_NUMBER] = {1, 2, 3, 4, 5};
    t_ilm_surface mp_ilmScreenIds[MAX_NUMBER] = {10, 20, 30, 40, 50};
    t_ilm_surface mp_ilmLayerIds[MAX_NUMBER] = {100, 200, 300, 400, 500};
    struct surface_context *mp_ctxSurface[MAX_NUMBER] = {nullptr};
    struct layer_context *mp_ctxLayer[MAX_NUMBER] = {nullptr};
    struct screen_context *mp_ctxScreen[MAX_NUMBER] = {nullptr};
    uint8_t m_iviWmControllerFakePointer = 0;
    struct ilmSurfaceProperties mp_surfaceProps[MAX_NUMBER] = {
        {0.6, 0, 0, 500, 500, 500, 500, 0, 0, 500, 500, ILM_TRUE, 10, 100, ILM_INPUT_DEVICE_ALL},
        {0.7, 10, 50, 600, 400, 600, 400, 50, 40, 200, 1000, ILM_FALSE, 30, 300, ILM_INPUT_DEVICE_POINTER|ILM_INPUT_DEVICE_KEYBOARD},
        {0.8, 20, 60, 700, 300, 700, 300, 60, 30, 300, 900, ILM_FALSE, 60, 1230, ILM_INPUT_DEVICE_KEYBOARD},
        {0.9, 30, 70, 800, 200, 800, 200, 70, 20, 400, 800, ILM_TRUE, 90, 4561, ILM_INPUT_DEVICE_KEYBOARD|ILM_INPUT_DEVICE_TOUCH},
        {1.0, 40, 80, 900, 100, 900, 100, 80, 10, 600, 700, ILM_TRUE, 100, 5646, ILM_INPUT_DEVICE_TOUCH},
    };
    struct ilmLayerProperties mp_layerProps[MAX_NUMBER] = {
        {0.1, 0, 0, 1280, 720, 0, 0, 1920, 1080, ILM_TRUE},
        {0.2, 10, 80, 1380, 520, 80, 10, 2920, 9080, ILM_FALSE},
        {0.3, 20, 70, 1480, 420, 70, 20, 3920, 8080, ILM_FALSE},
        {0.4, 30, 60, 1580, 320, 60, 30, 4920, 7080, ILM_FALSE},
        {0.5, 40, 50, 1680, 220, 50, 40, 5920, 6080, ILM_TRUE},
    };

    struct ilmScreenProperties mp_screenProps[MAX_NUMBER] = {
        {0, nullptr, 1920, 1080, "screen_1"},
        {0, nullptr, 3000, 10000, "screen_2"},
        {0, nullptr, 4000, 9000, "screen_3"},
        {0, nullptr, 5000, 8000, "screen_4"},
        {0, nullptr, 6000, 7000, "screen_5"},
    };

    int mp_successResult[1] = {0};
    int mp_failureResult[1] = {-1};
    ilmErrorTypes mp_ilmErrorType[1];
};

static void notificationCallback(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
    if (object == ILM_SURFACE) 
    {
        g_ilmControlStatus = created ? CREATE_SURFACE : DESTROY_SURFACE;
    } else if (object == ILM_LAYER) 
    {
        g_ilmControlStatus = created ? CREATE_LAYER : DESTROY_LAYER;
    }
}

TEST_F(IlmControlTest, ilm_getPropertiesOfSurface_invalidInput)
{
    // Invoke the ilm_getPropertiesOfSurface with wrong input, expect return ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfSurface(MAX_NUMBER + 1, nullptr));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should not trigger
    ASSERT_EQ(0, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(0, wl_display_roundtrip_queue_fake.call_count);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfSurface_cannotGetRoundTripQueue)
{
    // Prepare fake for wl_display_roundtrip_queue, to return failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getPropertiesOfSurface, expect a failure
    struct ilmSurfaceProperties l_surfaceProp = {};
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfSurface(1, &l_surfaceProp));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfSurface_cannotGetSurface)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getPropertiesOfSurface with wrong sureface id, expect failure
    struct ilmSurfaceProperties l_surfaceProp = {};
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfSurface(MAX_NUMBER + 1, &l_surfaceProp));\
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_successResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfSurface_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getPropertiesOfSurface with right input, expect success
    struct ilmSurfaceProperties l_surfaceProp = {};
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfSurface(1, &l_surfaceProp));
    // The surface properties output should same with prepare data
    ASSERT_EQ(l_surfaceProp.opacity, mp_surfaceProps[0].opacity);
    ASSERT_EQ(l_surfaceProp.sourceX, mp_surfaceProps[0].sourceX);
    ASSERT_EQ(l_surfaceProp.sourceY, mp_surfaceProps[0].sourceY);
    ASSERT_EQ(l_surfaceProp.sourceWidth, mp_surfaceProps[0].sourceWidth);
    ASSERT_EQ(l_surfaceProp.sourceHeight, mp_surfaceProps[0].sourceHeight);
    ASSERT_EQ(l_surfaceProp.origSourceWidth, mp_surfaceProps[0].origSourceWidth);
    ASSERT_EQ(l_surfaceProp.origSourceHeight, mp_surfaceProps[0].origSourceHeight);
    ASSERT_EQ(l_surfaceProp.destX, mp_surfaceProps[0].destX);
    ASSERT_EQ(l_surfaceProp.destY, mp_surfaceProps[0].destY);
    ASSERT_EQ(l_surfaceProp.destWidth, mp_surfaceProps[0].destWidth);
    ASSERT_EQ(l_surfaceProp.destHeight, mp_surfaceProps[0].destHeight);
    ASSERT_EQ(l_surfaceProp.visibility, mp_surfaceProps[0].visibility);
    ASSERT_EQ(l_surfaceProp.frameCounter, mp_surfaceProps[0].frameCounter);
    ASSERT_EQ(l_surfaceProp.creatorPid, mp_surfaceProps[0].creatorPid);
    ASSERT_EQ(l_surfaceProp.focus, mp_surfaceProps[0].focus);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfLayer_invalidInput)
{
    // Invoke the ilm_getPropertiesOfLayer with wrong input, expect return ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfLayer((MAX_NUMBER + 1) *100, nullptr));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should not trigger
    ASSERT_EQ(0, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(0, wl_display_roundtrip_queue_fake.call_count);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfLayer_cannotGetRoundTripQueue)
{
    // Prepare fake for wl_display_roundtrip_queue, to return failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getPropertiesOfLayer, expect a failure
    struct ilmLayerProperties l_layerProp = {};
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfLayer(100, &l_layerProp));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfLayer_cannotGetLayer)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getPropertiesOfLayer, with wrong layer id, expect a failure
    struct ilmLayerProperties l_layerProp = {};
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfLayer((MAX_NUMBER + 1) *100, &l_layerProp));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_successResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfLayer_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getPropertiesOfLayer, with right input, expect a success
    struct ilmLayerProperties l_layerProp = {};
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfLayer(100, &l_layerProp));
    // The layer properties should same with prepare
    ASSERT_EQ(l_layerProp.opacity, mp_layerProps[0].opacity);
    ASSERT_EQ(l_layerProp.sourceX, mp_layerProps[0].sourceX);
    ASSERT_EQ(l_layerProp.sourceY, mp_layerProps[0].sourceY);
    ASSERT_EQ(l_layerProp.sourceWidth, mp_layerProps[0].sourceWidth);
    ASSERT_EQ(l_layerProp.sourceHeight, mp_layerProps[0].sourceHeight);
    ASSERT_EQ(l_layerProp.destX, mp_layerProps[0].destX);
    ASSERT_EQ(l_layerProp.destY, mp_layerProps[0].destY);
    ASSERT_EQ(l_layerProp.destWidth, mp_layerProps[0].destWidth);
    ASSERT_EQ(l_layerProp.destHeight, mp_layerProps[0].destHeight);
    ASSERT_EQ(l_layerProp.visibility, mp_layerProps[0].visibility);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfScreen_invalidInput)
{
    // Invoke the ilm_getPropertiesOfLayer with wrong input, expect return ILM_FAILED
    struct ilmScreenProperties l_ScreenProp = {};
    ASSERT_EQ(ILM_ERROR_INVALID_ARGUMENTS, ilm_getPropertiesOfScreen((MAX_NUMBER + 1) *10, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfScreen((MAX_NUMBER + 1) *10, &l_ScreenProp));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should not trigger
    ASSERT_EQ(0, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(0, wl_display_roundtrip_queue_fake.call_count);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfScreen_cannotGetRoundTripQueue)
{
    // Prepare fake for wl_display_roundtrip_queue, to return failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getPropertiesOfScreen, expected the failure
    struct ilmScreenProperties l_ScreenProp = {};
    ASSERT_EQ(ILM_FAILED, ilm_getPropertiesOfScreen(10, &l_ScreenProp));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getPropertiesOfScreen_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Prepare the data output of ilm_getPropertiesOfScreen
    uint32_t *l_addLayer = (uint32_t*)custom_wl_array_add(&mp_ctxScreen[0]->render_order, sizeof(uint32_t));
    *l_addLayer = 100;
    l_addLayer = (uint32_t*)custom_wl_array_add(&mp_ctxScreen[0]->render_order, sizeof(uint32_t));
    *l_addLayer = 200;
    // Invoke the ilm_getPropertiesOfScreen, with right input, expect the success
    struct ilmScreenProperties l_ScreenProp = {};
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfScreen(10, &l_ScreenProp));
    // properties screen output should same with preapre data
    EXPECT_EQ(l_ScreenProp.layerCount, 2);
    EXPECT_EQ(l_ScreenProp.layerIds[0], 100);
    EXPECT_EQ(l_ScreenProp.layerIds[1], 200);
    EXPECT_EQ(l_ScreenProp.screenWidth, mp_ctxScreen[0]->prop.screenWidth);
    EXPECT_EQ(l_ScreenProp.screenHeight, mp_ctxScreen[0]->prop.screenHeight);
    EXPECT_EQ(0, strcmp(l_ScreenProp.connectorName, mp_ctxScreen[0]->prop.connectorName));
    // free resource
    free(l_ScreenProp.layerIds);
    custom_wl_array_release(&mp_ctxScreen[0]->render_order);
}

TEST_F(IlmControlTest, ilm_getScreenIDs_cannotSyncAcquireInstance)
{
    t_ilm_uint l_numberIds = 0;
    t_ilm_uint *lp_listIds = nullptr;

    // Set the ilm context initialized is false, not ready init
    ilm_context.initialized = false;
    // Invoke the ilm_getScreenIDs, expected the ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_getScreenIDs(&l_numberIds, &lp_listIds));
    // Set the ilm context initialized is true and prepare fake for wl_display_roundtrip_queue, to return faileure
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getScreenIDs, expeted return failure
    ASSERT_EQ(ILM_FAILED, ilm_getScreenIDs(&l_numberIds, &lp_listIds));
    // wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getScreenIDs_invaildInput)
{
    // Prepare fake for wl_display_roundtrip_queue and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke to ilm_getScreenIDs, with invalid input, expect the failures
    t_ilm_uint l_numberIds = 0;
    t_ilm_uint *lp_listIds = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getScreenIDs(nullptr, &lp_listIds));
    ASSERT_EQ(ILM_FAILED, ilm_getScreenIDs(&l_numberIds, nullptr));
}

TEST_F(IlmControlTest, ilm_getScreenIDs_success)
{
    // Prepare fake for wl_display_roundtrip_queue and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke to ilm_getScreenIDs, with right input, expect the success
    t_ilm_uint l_numberIds = 0;
    t_ilm_uint *lp_listIds = nullptr;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&l_numberIds, &lp_listIds));
    // The result output should same with the prepare input
    EXPECT_EQ(MAX_NUMBER, l_numberIds);
    for(uint8_t i = 0; i< l_numberIds; i++)
    {
        EXPECT_EQ(lp_listIds[i], mp_ilmScreenIds[i]);
    }
    // free resource
    free(lp_listIds);
}

TEST_F(IlmControlTest, ilm_getScreenResolution_cannotSyncAcquireInstance)
{
    t_ilm_uint l_screenWidth = 0, l_screenHeight = 0;
    // Set the ilm context initialized is false, not ready init
    ilm_context.initialized = false;
    // Invoke the ilm_getScreenResolution, expected the ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_getScreenResolution(10, &l_screenWidth, &l_screenHeight));
    // Prepare fake for wl_display_roundtrip_queue return a failure and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getScreenResolution and expected the failure
    ASSERT_EQ(ILM_FAILED, ilm_getScreenResolution(10, &l_screenWidth, &l_screenHeight));
    // wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getScreenResolution_invaildInput)
{
    // Prepare fake for wl_display_roundtrip_queue return a success and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getScreenResolution with invalid input, expected the failures
    t_ilm_uint l_screenWidth = 0, l_screenHeight = 0;
    ASSERT_EQ(ILM_FAILED, ilm_getScreenResolution(10, nullptr, &l_screenHeight));
    ASSERT_EQ(ILM_FAILED, ilm_getScreenResolution(10, &l_screenWidth, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getScreenResolution(1, &l_screenWidth, &l_screenHeight));
}

TEST_F(IlmControlTest, ilm_getScreenResolution_success)
{
    // Prepare fake for wl_display_roundtrip_queue return a success and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getScreenResolution with right input, expected a success
    t_ilm_uint l_screenWidth = 0, l_screenHeight = 0;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenResolution(10, &l_screenWidth, &l_screenHeight));
    // The result output should same with prepare data
    EXPECT_EQ(1920, l_screenWidth);
    EXPECT_EQ(1080, l_screenHeight);
}

TEST_F(IlmControlTest, ilm_getLayerIDs_cannotSyncAcquireInstance)
{
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    // Set the ilm context initialized is false, not ready init
    ilm_context.initialized = false;
    // Invoke the ilm_getLayerIDs, expected a failure
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDs(&l_numberLayers, &lp_listLayers));
    // Prepare fake for wl_display_roundtrip_queue return a failure and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getLayerIDs, expected a failure
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDs(&l_numberLayers, &lp_listLayers));
    // The wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getLayerIDs_invaildInput)
{
    // Prepare fake for wl_display_roundtrip_queue return a success and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getLayerIDs with invalid input, expected the failures
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDs(nullptr, &lp_listLayers));
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDs(&l_numberLayers, nullptr));
}

TEST_F(IlmControlTest, ilm_getLayerIDs_success)
{
    // Prepare fake for wl_display_roundtrip_queue return a success and setup ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getLayerIDs with right input data, expected a success
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&l_numberLayers, &lp_listLayers));
    // The result output shoud same with prepare data
    EXPECT_EQ(MAX_NUMBER, l_numberLayers);
    for(uint8_t i = 0; i< l_numberLayers; i++)
    {
        EXPECT_EQ(lp_listLayers[i], mp_ilmLayerIds[i]);
    }
    // free resource
    free(lp_listLayers);
}

TEST_F(IlmControlTest, ilm_getLayerIDsOnScreen_invalidInput)
{
    // Invoke the ilm_getLayerIDsOnScreen with invalid input, expected the failures
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDsOnScreen(10, &l_numberLayers, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDsOnScreen(10, nullptr, &lp_listLayers));
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDsOnScreen(1, &l_numberLayers, &lp_listLayers));
}

TEST_F(IlmControlTest, ilm_getLayerIDsOnScreen_cannotGetRoundTripQueue)
{
    // Prepare fake for wl_display_roundtrip_queue, to make return failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getLayerIDsOnScreen, expected a failure
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getLayerIDsOnScreen(10, &l_numberLayers, &lp_listLayers));
    // The wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getLayerIDsOnScreen_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to make return success
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Prepare the data output
    uint32_t *l_addLayer = (uint32_t*)custom_wl_array_add(&mp_ctxScreen[0]->render_order, sizeof(uint32_t));
    *l_addLayer = 100;
    l_addLayer = (uint32_t*)custom_wl_array_add(&mp_ctxScreen[0]->render_order, sizeof(uint32_t));
    *l_addLayer = 200;
    // Invoke ilm_getLayerIDsOnScreen with right input, expected the success
    t_ilm_int l_numberLayers = 0;
    t_ilm_layer *lp_listLayers = nullptr;
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDsOnScreen(10, &l_numberLayers, &lp_listLayers));
    // The result data should same with preapre data
    EXPECT_EQ(l_numberLayers, 2);
    EXPECT_EQ(lp_listLayers[0], 100);
    EXPECT_EQ(lp_listLayers[1], 200);
    //free resource
    free(lp_listLayers);
    custom_wl_array_release(&mp_ctxScreen[0]->render_order);
}

TEST_F(IlmControlTest, ilm_getSurfaceIDs_cannotSyncAcquireInstance)
{
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_surface *lp_listSurfaces = nullptr;
    // Prepare the ilm context initialized is not ready init
    ilm_context.initialized = false;
    // Invoke the ilm_getSurfaceIDs, expected the failure
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDs(&l_numberSurfaces, &lp_listSurfaces));
    // Prepare fake for wl_display_roundtrip_queue, to return failure, and ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getSurfaceIDs and expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDs(&l_numberSurfaces, &lp_listSurfaces));
    // The wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getSurfaceIDs_invaildInput)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success, and ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getSurfaceIDs with invalid input, expected the failures
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_surface *lp_listSurfaces = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDs(nullptr, &lp_listSurfaces));
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDs(&l_numberSurfaces, nullptr));
}

TEST_F(IlmControlTest, ilm_getSurfaceIDs_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to return success, and ilm context is initialized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_getSurfaceIDs with right data input, expect a success
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_surface *lp_listSurfaces = nullptr;
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDs(&l_numberSurfaces, &lp_listSurfaces));
    // The result output should same with prepare data
    EXPECT_EQ(MAX_NUMBER, l_numberSurfaces);
    for(uint8_t i = 0; i< l_numberSurfaces; i++)
    {
        EXPECT_EQ(lp_listSurfaces[i], mp_ilmSurfaceIds[i]);
    }
    //free resource
    free(lp_listSurfaces);
}

TEST_F(IlmControlTest, ilm_getSurfaceIDsOnLayer_invalidInput)
{
    // Invoke the ilm_getSurfaceIDsOnLayer with invalied input, expect the failures
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_layer *lp_listSurfaces = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDsOnLayer(100, &l_numberSurfaces, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDsOnLayer(100, nullptr, &lp_listSurfaces));
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDsOnLayer(1, &l_numberSurfaces, &lp_listSurfaces));
}

TEST_F(IlmControlTest, ilm_getSurfaceIDsOnLayer_cannotGetRoundTripQueue)
{
    // Prepare fake for wl_display_roundtrip_queue, to return the failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_getSurfaceIDsOnLayer, expect the failure
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_layer *lp_listSurfaces = nullptr;
    ASSERT_EQ(ILM_FAILED, ilm_getSurfaceIDsOnLayer(100, &l_numberSurfaces, &lp_listSurfaces));
    // wl_proxy_marshal_flags and wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_getSurfaceIDsOnLayer_success)
{
    // Prepare fake for wl_display_roundtrip_queue, to return the failure
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Prepare the data output
    uint32_t *l_addSurface = (uint32_t*)custom_wl_array_add(&mp_ctxLayer[0]->render_order, sizeof(uint32_t));
    *l_addSurface = 1;
    l_addSurface = (uint32_t*)custom_wl_array_add(&mp_ctxLayer[0]->render_order, sizeof(uint32_t));
    *l_addSurface = 2;
    // Invoke the ilm_getSurfaceIDsOnLayer with right input, expected the success
    t_ilm_int l_numberSurfaces = 0;
    t_ilm_layer *lp_listSurfaces = nullptr;
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(100, &l_numberSurfaces, &lp_listSurfaces));
    // The result output should same with prepare data
    EXPECT_EQ(l_numberSurfaces, 2);
    EXPECT_EQ(lp_listSurfaces[0], 1);
    EXPECT_EQ(lp_listSurfaces[1], 2);
    // free resource
    free(lp_listSurfaces);
    custom_wl_array_release(&mp_ctxLayer[0]->render_order);
}

TEST_F(IlmControlTest, ilm_layerCreateWithDimension_cannotSyncAcquireInstance)
{
    t_ilm_layer l_layerId = 600;
    // Setup the ilm context is not initilized
    ilm_context.initialized = false;
    // Invoke the ilm_layerCreateWithDimension, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_layerCreateWithDimension(&l_layerId, 640, 480));
    // Prepare fake for ilm_layerCreateWithDimension, to return failure and ilm context is initilized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_failureResult, 1);
    // Invoke the ilm_layerCreateWithDimension and expect the failure
    ASSERT_EQ(ILM_FAILED, ilm_layerCreateWithDimension(&l_layerId, 640, 480));
    // The wl_display_roundtrip_queue should trigger once time
    ASSERT_EQ(1, wl_display_roundtrip_queue_fake.call_count);
    ASSERT_EQ(mp_failureResult[0], wl_display_roundtrip_queue_fake.return_val_history[0]);
}

TEST_F(IlmControlTest, ilm_layerCreateWithDimension_invaildInput)
{
    // Prepare fake for ilm_layerCreateWithDimension, to return success and ilm context is initilized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_layerCreateWithDimension with invalid input data, expect return failure
    t_ilm_layer l_layerId = 100;
    ASSERT_EQ(ILM_FAILED, ilm_layerCreateWithDimension(nullptr, 640, 480));
    ASSERT_EQ(ILM_FAILED, ilm_layerCreateWithDimension(&l_layerId, 640, 480));
}

TEST_F(IlmControlTest, ilm_layerCreateWithDimension_success)
{
    // Prepare fake for ilm_layerCreateWithDimension, to return success and ilm context is initilized
    ilm_context.initialized = true;
    SET_RETURN_SEQ(wl_display_roundtrip_queue, mp_successResult, 1);
    // Invoke the ilm_layerCreateWithDimension with id doesn't exist, expected a success
    t_ilm_layer l_layerId = 600;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&l_layerId, 640, 480));
    // Invoke the ilm_layerCreateWithDimension with id INVALID, expected a success
    l_layerId = INVALID_ID;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&l_layerId, 640, 480));
    // With input is invalid id, expectd the layer id is set 0
    ASSERT_EQ(0, l_layerId);
}

TEST_F(IlmControlTest, wm_listener_layer_created_sameLayerId)
{
    // invoke the wm_listener_layer_created with layer id exist, expect there is no the ctx_layer create
    wm_listener_layer_created(&ilm_context.wl, nullptr, 100);
    // The wl_list_insert should not trigger
    ASSERT_EQ(0, wl_list_insert_fake.call_count);
}

TEST_F(IlmControlTest, wm_listener_layer_created_addNewOne)
{
    // Invoke the wm_listener_layer_created with new layer id, expect a new ctx_layer created
    wm_listener_layer_created(&ilm_context.wl, nullptr, 600);
    // The wl_list_insert should trigger
    ASSERT_EQ(1, wl_list_insert_fake.call_count);
    // free resource
    struct layer_context *lp_createLayer = (struct layer_context*)(uintptr_t(wl_list_insert_fake.arg1_history[0]) - offsetof(struct layer_context, link));
    free(lp_createLayer);

    // Invoke the wm_listener_layer_created with new layer id and there is a callback notification, expect new ctx_layer created and callback is trigged
    ilm_context.wl.notification = notificationCallback;
    wm_listener_layer_created(&ilm_context.wl, nullptr, 600);
    // The The wl_list_insert should trigger and callback is trigged
    ASSERT_EQ(2, wl_list_insert_fake.call_count);
    ASSERT_EQ(CREATE_LAYER, g_ilmControlStatus);
    // free resource
    lp_createLayer = (struct layer_context*)(uintptr_t(wl_list_insert_fake.arg1_history[1]) - offsetof(struct layer_context, link));
    free(lp_createLayer);
}

TEST_F(IlmControlTest, wm_listener_layer_destroyed_wrongLayerId)
{
    // Invoke the wm_listener_layer_destroyed with wrong layer id, expect there isn't layer will remove
    wm_listener_layer_destroyed(&ilm_context.wl, nullptr, 1);
    // The wl_list_remove should not trigger
    ASSERT_EQ(0, wl_list_remove_fake.call_count);
}

TEST_F(IlmControlTest, wm_listener_layer_destroyed_removeOne)
{
    // Prepare fake for wl_list_remove, to remove real object
    wl_list_remove_fake.custom_fake = custom_wl_list_remove;
    // Invoke to wm_listener_layer_destroyed with 
    wm_listener_layer_destroyed(&ilm_context.wl, nullptr, 100);
    // wl_list_remove should trigger once time
    ASSERT_EQ(1, wl_list_remove_fake.call_count);
    mp_ctxLayer[0] = nullptr;

    // Invoke the wm_listener_layer_destroyed with a callback register
    ilm_context.wl.notification = notificationCallback;
    wm_listener_layer_destroyed(&ilm_context.wl, nullptr, 200);
    // The wl_list_remove should trigger and notification callback should called
    ASSERT_EQ(2, wl_list_remove_fake.call_count);
    ASSERT_EQ(DESTROY_LAYER, g_ilmControlStatus);
    mp_ctxLayer[1] = nullptr;
}

TEST_F(IlmControlTest, wm_listener_layer_surface_added_wrongLayerId)
{
    // Invoke the wm_listener_layer_surface_added with exist id, expect there isn't a id add to array
    wm_listener_layer_surface_added(&ilm_context.wl, nullptr, 1, 1);
    // wl_array_add should not trigger
    ASSERT_EQ(0, wl_array_add_fake.call_count);
}

TEST_F(IlmControlTest, wm_listener_layer_surface_added_addNewOne)
{
    // Prepare fake for wl_array_add
    uint32_t l_dataSurface = 0;
    void *lp_elemData = &l_dataSurface;
    SET_RETURN_SEQ(wl_array_add, &lp_elemData, 1);
    // Invoke the wm_listener_layer_surface_added
    wm_listener_layer_surface_added(&ilm_context.wl, nullptr, 100, 1);
    // The wl_array_add should trigger once time and surface id should assign to member of array
    ASSERT_EQ(1, wl_array_add_fake.call_count);
    ASSERT_EQ(1, l_dataSurface);
}

TEST_F(IlmControlTest, wm_listener_surface_created_sameSurfaceId)
{
    // Invoke the wm_listener_surface_created with surface id exist, expect there isn't the ctx_layer created
    wm_listener_surface_created(&ilm_context.wl, nullptr, 1);
    // wl_list_insert should not trigger
    ASSERT_EQ(0, wl_list_insert_fake.call_count);
}

TEST_F(IlmControlTest, wm_listener_surface_created_addNewOne)
{
    // Invoke the wm_listener_surface_created with exist surface id in list, expect there is a id add to list
    wm_listener_surface_created(&ilm_context.wl, nullptr, MAX_NUMBER + 1);
    // wl_list_insert should not trigger
    ASSERT_EQ(1, wl_list_insert_fake.call_count);
    // free resource
    struct surface_context *lp_createSurface = (struct surface_context*)(uintptr_t(wl_list_insert_fake.arg1_history[0]) - offsetof(struct surface_context, link));
    free(lp_createSurface);

    // Invoke the wm_listener_surface_created with new layer id and there is a callback notification, expect new ctx_layer created and callback is trigged
    ilm_context.wl.notification = notificationCallback;
    wm_listener_surface_created(&ilm_context.wl, nullptr, MAX_NUMBER + 1);
    // The The wl_list_insert should trigger and callback is trigged
    ASSERT_EQ(2, wl_list_insert_fake.call_count);
    ASSERT_EQ(CREATE_SURFACE, g_ilmControlStatus);
    // free resource
    lp_createSurface = (struct surface_context*)(uintptr_t(wl_list_insert_fake.arg1_history[1]) - offsetof(struct surface_context, link));
    free(lp_createSurface);
}

TEST_F(IlmControlTest, wm_listener_surface_destroyed_wrongSurfaceId)
{
    // Invoke the wm_listener_surface_destroyed with wrong surface id, expect there isn't surface will remove
    wm_listener_surface_destroyed(&ilm_context.wl, nullptr, MAX_NUMBER + 1);
    // The wl_list_remove should not trigger
    ASSERT_EQ(0, wl_list_remove_fake.call_count);
}

TEST_F(IlmControlTest, wm_listener_surface_destroyed_removeOne)
{
    // Prepare fake for wl_list_remove, to remove real object
    wl_list_remove_fake.custom_fake = custom_wl_list_remove;
    // Invoke to wm_listener_surface_destroyed with a surface id, expect there is a surface will remove
    ilm_context.wl.notification = NULL;
    wm_listener_surface_destroyed(&ilm_context.wl, nullptr, 1);
    // wl_list_remove should trigger once time
    ASSERT_EQ(1, wl_list_remove_fake.call_count);
    mp_ctxLayer[0] = nullptr;

    // Invoke the wm_listener_surface_destroyed with a callback register
    ilm_context.wl.notification = notificationCallback;
    wm_listener_surface_destroyed(&ilm_context.wl, nullptr, 2);
    // The wl_list_remove should trigger and notification callback should called
    ASSERT_EQ(2, wl_list_remove_fake.call_count);
    ASSERT_EQ(DESTROY_SURFACE, g_ilmControlStatus);
    mp_ctxLayer[1] = nullptr;
}
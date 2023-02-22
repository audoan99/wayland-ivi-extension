#include <gtest/gtest.h>
#include "ilm_common.h"
#include "ilm_common_platform.h"
#include "client_api_fake.h"

extern "C"
{
    FAKE_VALUE_FUNC(ilmErrorTypes, ilmControl_init, t_ilm_nativedisplay);
    FAKE_VOID_FUNC(ilmControl_destroy);
    FAKE_VALUE_FUNC(ilmErrorTypes, ilmControl_registerShutdownNotification, shutdownNotificationFunc, void*);
}

class IlmCommonTest : public ::testing::Test 
{
public:
    void SetUp()
    {
        CLIENT_API_FAKE_LIST(RESET_FAKE);
        RESET_FAKE(ilmControl_init);
        RESET_FAKE(ilmControl_destroy);
        RESET_FAKE(ilmControl_registerShutdownNotification);
    }

    void TearDown()
    {
        if(ilm_isInitialized())
        {
            ilm_destroy();
        }
    }

    void mock_ilmInitSuccess()
    {
        mpp_wlDisplays[0] = (struct wl_display*)&m_wlDisplayFakePointer;
        SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);

        mp_ilmErrorType[0] = ILM_SUCCESS;
        SET_RETURN_SEQ(ilmControl_init, mp_ilmErrorType, MAX_NUMBER);
    }

    static constexpr uint8_t MAX_NUMBER = 1;
    uint8_t m_wlDisplayFakePointer = 0;
    struct wl_display* mpp_wlDisplays [MAX_NUMBER] = {nullptr};
    ilmErrorTypes mp_ilmErrorType[MAX_NUMBER] = {ILM_FAILED};
};

TEST_F(IlmCommonTest, ilm_init_cannotGetDisplay)
{
    // Prepare fake for wl_display_connect, to make the result is nullptr
    mpp_wlDisplays[0] = nullptr;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the ilm_init and expect the result is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_init());
    /* Check the logic
     * wl_display_connect should call one time and return nullptr 
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 1);
    ASSERT_EQ(wl_display_connect_fake.return_val_history[0], nullptr);
}

TEST_F(IlmCommonTest, ilm_init_getFailedOnilmControl_init)
{
    // Prepare fake for wl_display_connect, to make return different nullptr
    mpp_wlDisplays[0] = (struct wl_display*)&m_wlDisplayFakePointer;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Prepare fake for ilmControl_init, to make return ILM_FAILED
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(ilmControl_init, mp_ilmErrorType, MAX_NUMBER);
    // Invoke the ilmControl_init and expect the result is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_init());
    /* Check the logic
     * ilmControl_init_fake should call one time and return ILM_FAILED
     */
    ASSERT_EQ(ilmControl_init_fake.call_count, 1);
    ASSERT_EQ(mp_ilmErrorType[0], ilmControl_init_fake.return_val_history[0]);
}

TEST_F(IlmCommonTest, ilm_init_getSuccess)
{
    // Prepare fake for wl_display_connect and ilmControl_init, to make the ilm_init return success
    mock_ilmInitSuccess();
    // Invoke the ilm_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_init());
    /* Check the logic
     * wl_display_connect should call one time
     * ilmControl_init_fake should call one time and return ILM_SUCCESS
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 1);
    ASSERT_EQ(ilmControl_init_fake.call_count, 1);
    ASSERT_EQ(ILM_SUCCESS, ilmControl_init_fake.return_val_history[0]);
}

TEST_F(IlmCommonTest, ilm_init_manyTimes)
{
    // Prepare fake for wl_display_connect and ilmControl_init, to make the ilm_init return success
    mock_ilmInitSuccess();
    // Invoke the ilm_init in first time and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_init());
    // Invoke the ilm_init in second time and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_init());
    // Invoke the ilm_destroy in first time and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_destroy());
    // Invoke the ilm_destroy in second time and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_destroy());
    // Invoke the ilm_destroy in third time and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_FAILED, ilm_destroy());
}

TEST_F(IlmCommonTest, ilm_initWithNativedisplay_existDisplay)
{
    // Prepare fake for wl_display_connect and ilmControl_init, to make the ilm_init return success
    mock_ilmInitSuccess();
    // Invoke the ilm_initWithNativedisplay with exist display and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_initWithNativedisplay(1));
    /* Check the logic
     * wl_display_connect will not be called
     * ilmControl_init_fake should call one time and return ILM_SUCCESS
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 0);
    ASSERT_EQ(ilmControl_init_fake.call_count, 1);
    ASSERT_EQ(ILM_SUCCESS, ilmControl_init_fake.return_val_history[0]);
}

TEST_F(IlmCommonTest, ilm_isInitialized_getFalse)
{
    // Don't call ilm_init
    // Invoke the ilm_isInitialized and expect return a failed
    ASSERT_EQ(ILM_FALSE, ilm_isInitialized());
}

TEST_F(IlmCommonTest, ilm_isInitialized_getTrue)
{
    // Prepare fake for wl_display_connect and ilmControl_init, to make the ilm_init return success
    mock_ilmInitSuccess();
    ASSERT_EQ(ILM_SUCCESS, ilm_init());
    // Invoke the ilm_isInitialized and expect return a success
    ASSERT_EQ(ILM_TRUE, ilm_isInitialized());
}

TEST_F(IlmCommonTest, ilm_registerShutdownNotification_getFailure)
{
    // Prepare fake for ilmControl_registerShutdownNotification, to make return failed
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(ilmControl_registerShutdownNotification, mp_ilmErrorType, MAX_NUMBER);
    // Invoke the ilm_registerShutdownNotification and expect the failed
    ASSERT_EQ(ILM_FAILED, ilm_registerShutdownNotification(nullptr, nullptr));
    /* Check the logic
     * ilmControl_registerShutdownNotification should call one time and return ILM_FAILED
     */
    ASSERT_EQ(1, ilmControl_registerShutdownNotification_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], ilmControl_registerShutdownNotification_fake.return_val_history[0]);
}

TEST_F(IlmCommonTest, ilm_registerShutdownNotification_getSuccess)
{
    // Prepare fake for ilmControl_registerShutdownNotification, to make return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(ilmControl_registerShutdownNotification, mp_ilmErrorType, MAX_NUMBER);
    // Invoke the ilm_registerShutdownNotification and expect the success
    ASSERT_EQ(ILM_SUCCESS, ilm_registerShutdownNotification(nullptr, nullptr));
    /* Check the logic
     * ilmControl_registerShutdownNotification should call one time and return ILM_SUCCESS
     */
    ASSERT_EQ(1, ilmControl_registerShutdownNotification_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], ilmControl_registerShutdownNotification_fake.return_val_history[0]);
}

TEST_F(IlmCommonTest, ilm_destroy_getFailure)
{
    // Don't call ilm_init
    // Invoke the ilm_destroy and expect return ILM_FAILED
    ASSERT_EQ(ILM_FAILED, ilm_destroy());
}

TEST_F(IlmCommonTest, ilm_destroy_getSuccess)
{
    // Prepare fake for wl_display_connect and ilmControl_init, to make the ilm_init return success
    mock_ilmInitSuccess();
    ASSERT_EQ(ILM_SUCCESS, ilm_init());
    // Invoke the ilm_destroy and expect return ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, ilm_destroy());
}

TEST_F(IlmCommonTest, wayland_init_cannotGetDisplay)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Prepare fake for wl_display_connect, to make the result is nullptr
    mpp_wlDisplays[0] = nullptr;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, gIlmCommonPlatformFunc.init(0));
    /* Check the logic
     * wl_display_connect should call one time
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 1);
    ASSERT_EQ(wl_display_connect_fake.return_val_history[0], nullptr);
}

TEST_F(IlmCommonTest, wayland_init_existDisplay)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(1));
    /* Check the logic
     * wl_display_connect should not be called
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 0);
}

TEST_F(IlmCommonTest, wayland_init_getSuccess)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Prepare fake for wl_display_connect, to make the result is success
    mpp_wlDisplays[0] = (struct wl_display*)&m_wlDisplayFakePointer;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(0));
    /* Check the logic
     * wl_display_connect should call one time
     */
    ASSERT_EQ(wl_display_connect_fake.call_count, 1);
    ASSERT_EQ(wl_display_connect_fake.return_val_history[0], mpp_wlDisplays[0]);
}

TEST_F(IlmCommonTest, wayland_getNativedisplay_getValue)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();

    // Prepare fake for wl_display_connect, to make the result is nullptr
    mpp_wlDisplays[0] = nullptr;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, gIlmCommonPlatformFunc.init(0));
    // Invoke the wayland_getNativedisplay and expect the return is null (0)
    ASSERT_EQ(0, gIlmCommonPlatformFunc.getNativedisplay());

    // Prepare fake for wl_display_connect, to make the result is success
    mpp_wlDisplays[0] = (struct wl_display*)&m_wlDisplayFakePointer;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(0));
    // Invoke the wayland_getNativedisplay and expect the return is mpp_wlDisplays
    ASSERT_EQ(mpp_wlDisplays[0], gIlmCommonPlatformFunc.getNativedisplay());
    
    // Invoke the wayland_init with exist display and expect the result is this display
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(1));
    ASSERT_EQ(1, gIlmCommonPlatformFunc.getNativedisplay());
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(2));
    ASSERT_EQ(2, gIlmCommonPlatformFunc.getNativedisplay());
}

TEST_F(IlmCommonTest, wayland_isInitialized_getFalse)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Prepare fake for wl_display_connect, to make the result is nullptr
    mpp_wlDisplays[0] = nullptr;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, gIlmCommonPlatformFunc.init(0));
    // Invoke the wayland_isInitialized and expect the return is false
    ASSERT_EQ(0, gIlmCommonPlatformFunc.isInitialized());
}

TEST_F(IlmCommonTest, wayland_isInitialized_getTrue)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(1));
    // Invoke the wayland_isInitialized and expect the return is true
    ASSERT_EQ(1, gIlmCommonPlatformFunc.isInitialized());
}

TEST_F(IlmCommonTest, wayland_destroy_cannotGetDisplay)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Prepare fake for wl_display_connect, to make the result is nullptr
    mpp_wlDisplays[0] = nullptr;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_FAILED
    ASSERT_EQ(ILM_FAILED, gIlmCommonPlatformFunc.init(0));
    // Invoke the wayland_destroy and expect the return is false
    ASSERT_EQ(ILM_FAILED, gIlmCommonPlatformFunc.destroy());
    /* Check the logic
     * wl_display_roundtrip should not be called
     * wl_display_disconnect should not be called
     */
    ASSERT_EQ(wl_display_roundtrip_fake.call_count, 0);
    ASSERT_EQ(wl_display_disconnect_fake.call_count, 0);
}

TEST_F(IlmCommonTest, wayland_destroy_existDisplay)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(1));
    // Invoke the wayland_destroy and expect the return is true
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.destroy());
    /* Check the logic
     * wl_display_roundtrip should not be called
     * wl_display_disconnect should not be called
     */
    ASSERT_EQ(wl_display_roundtrip_fake.call_count, 0);
    ASSERT_EQ(wl_display_disconnect_fake.call_count, 0);
}

TEST_F(IlmCommonTest, wayland_destroy_getSuccess)
{
    // Get address functions to gIlmCommonPlatformFunc
    init_ilmCommonPlatformTable();
    // Prepare fake for wl_display_connect, to make the result is success
    mpp_wlDisplays[0] = (struct wl_display*)&m_wlDisplayFakePointer;
    SET_RETURN_SEQ(wl_display_connect, mpp_wlDisplays, MAX_NUMBER);
    // Invoke the wayland_init and expect the resutl is ILM_SUCCESS
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.init(0));
    // Invoke the wayland_destroy and expect the return is true
    ASSERT_EQ(ILM_SUCCESS, gIlmCommonPlatformFunc.destroy());
    /* Check the logic
     * wl_display_roundtrip should call one time
     * wl_display_disconnect should call one time
     */
    ASSERT_EQ(wl_display_roundtrip_fake.call_count, 1);
    ASSERT_EQ(wl_display_disconnect_fake.call_count, 1);    
}
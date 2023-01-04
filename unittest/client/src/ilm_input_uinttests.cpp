#include <gtest/gtest.h>
#include "wayland-util.h"
#include "ilm_input.h"
#include "ilm_control_platform.h"
#include "client_api_fake.h"

extern "C"{
    extern struct ilm_control_context ilm_context;
    FAKE_VALUE_FUNC(ilmErrorTypes, impl_sync_and_acquire_instance, struct ilm_control_context *);
    FAKE_VOID_FUNC(release_instance);
}

class IlmInputTest : public ::testing::Test 
{
public:
    void SetUp()
    {
        CLIENT_API_FAKE_LIST(RESET_FAKE);
        RESET_FAKE(impl_sync_and_acquire_instance);
        RESET_FAKE(release_instance);

        init_ctx_list_content();
    }

    void TearDown()
    {
        deinit_ctx_list_content();
    }

    void init_ctx_list_content()
    {
        custom_wl_list_init(&ilm_context.wl.list_surface);
        custom_wl_list_init(&ilm_context.wl.list_seat);
        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            //prepare the surfaces
            mp_ctxSurface[i].id_surface = mp_ilmSurfaceIds[i];
            mp_ctxSurface[i].ctx = &ilm_context.wl;
            mp_ctxSurface[i].prop.focus = ILM_INPUT_DEVICE_KEYBOARD;
            custom_wl_list_insert(&ilm_context.wl.list_surface, &mp_ctxSurface[i].link);
            custom_wl_list_init(&mp_ctxSurface[i].list_accepted_seats);
            //prepare the accepted seat
            mp_acceptedSeat[i].seat_name = (char*)mp_seatName[i];
            custom_wl_list_insert(&mp_ctxSurface[i].list_accepted_seats, &mp_acceptedSeat[i].link);
            //prepare the seats
            mp_ctxSeat[i].capabilities = ILM_INPUT_DEVICE_ALL;
            mp_ctxSeat[i].seat_name = (char*)mp_seatName[i];
            custom_wl_list_insert(&ilm_context.wl.list_seat, &mp_ctxSeat[i].link);
        }
    }

    void deinit_ctx_list_content()
    {
        struct surface_context *l, *n;
        struct accepted_seat *seat, *seat_next;
        wl_list_for_each_safe(l, n, &ilm_context.wl.list_surface, link) 
        {
            wl_list_for_each_safe(seat, seat_next, &l->list_accepted_seats, link) 
            {
                custom_wl_list_remove(&seat->link);
            }
            custom_wl_list_remove(&l->link);
        }
    }

    static constexpr uint8_t MAX_NUMBER = 5;
    const char *SEAT_DEFAULT = "seat_not_found";

    t_ilm_surface mp_ilmSurfaceIds[MAX_NUMBER] = {1, 2, 3, 4, 5};
    const char *mp_seatName[MAX_NUMBER] = {"seat_1", "seat_2", "seat_3", "seat_4", "seat_5"};
    struct surface_context mp_ctxSurface[MAX_NUMBER] = {};
    struct seat_context mp_ctxSeat[MAX_NUMBER] = {};
    struct accepted_seat mp_acceptedSeat[MAX_NUMBER] = {};

    ilmErrorTypes mp_ilmErrorType[MAX_NUMBER] = {ILM_FAILED};

    t_ilm_string *mp_getSeats = nullptr;
    t_ilm_uint m_numberSeat = 0;
}; //IlmInputTest

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_invalidAgrument)
{
    // Invoke to ilm_setInputAcceptanceOn, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_setInputAcceptanceOn(mp_ilmSurfaceIds[0], 1, nullptr));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);
}

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failed
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputAcceptanceOn, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_setInputAcceptanceOn(mp_ilmSurfaceIds[0], 1, (t_ilm_string*)&SEAT_DEFAULT));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_cannotFindSurfaceId)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputAcceptanceOn, with wrong input surface id, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_setInputAcceptanceOn(MAX_NUMBER + 1, 1, (t_ilm_string*)&SEAT_DEFAULT));
    // release_instance should trigger once time
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_cannotFindSeat)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputAcceptanceOn, with non-exist seat, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_setInputAcceptanceOn(mp_ilmSurfaceIds[0], 1, (t_ilm_string*)&SEAT_DEFAULT));
    // release_instance should trigger once time
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_addNewOne)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputAcceptanceOn, with right input, expect a success
    ASSERT_EQ(ILM_SUCCESS, ilm_setInputAcceptanceOn(mp_ilmSurfaceIds[0], 2, (t_ilm_string*)&mp_seatName));
    // set input accepted should be sent once time, release instance should trigger
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputAcceptanceOn_removeOne)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputAcceptanceOn, with right input, expect a success
    ASSERT_EQ(ILM_SUCCESS, ilm_setInputAcceptanceOn(mp_ilmSurfaceIds[0], 1, (t_ilm_string*)&mp_seatName[1]));
    // set input accepted should be sent two time, release instance should trigger
    ASSERT_EQ(2, wl_proxy_marshal_flags_fake.call_count);
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_getInputAcceptanceOn_invalidAgrument)
{
    // Invoke the ilm_getInputAcceptanceOn with wrong input, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputAcceptanceOn(mp_ilmSurfaceIds[0], &m_numberSeat, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getInputAcceptanceOn(mp_ilmSurfaceIds[0], nullptr, nullptr));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);
}

TEST_F(IlmInputTest, ilm_getInputAcceptanceOn_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failure
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputAcceptanceOn, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputAcceptanceOn(mp_ilmSurfaceIds[0], &m_numberSeat, &mp_getSeats));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], impl_sync_and_acquire_instance_fake.return_val_history[0]);
}

TEST_F(IlmInputTest, ilm_getInputAcceptanceOn_cannotFindSurfaceId)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputAcceptanceOn with non-exist surface id, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputAcceptanceOn(MAX_NUMBER + 1, &m_numberSeat, &mp_getSeats));
    // release_instance should trigger once time
    ASSERT_EQ(release_instance_fake.call_count, 1);
}

TEST_F(IlmInputTest, ilm_getInputAcceptanceOn_success)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputAcceptanceOn with right inpur, expect a success
    ASSERT_EQ(ILM_SUCCESS, ilm_getInputAcceptanceOn(mp_ilmSurfaceIds[0], &m_numberSeat, &mp_getSeats));
    // output should same with prepare data
    EXPECT_EQ(1, m_numberSeat);
    EXPECT_EQ(0, strcmp(mp_getSeats[0], "seat_1"));
    EXPECT_EQ(1, release_instance_fake.call_count);
    // free resource
    free(mp_getSeats[0]);
    free(mp_getSeats);
}

TEST_F(IlmInputTest, ilm_getInputDevices_invalidAgrument)
{
    // Invoke the ilm_getInputDevices with wrong input, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputDevices(ILM_INPUT_DEVICE_KEYBOARD, &m_numberSeat, nullptr));
    ASSERT_EQ(ILM_FAILED, ilm_getInputDevices(ILM_INPUT_DEVICE_KEYBOARD, nullptr, nullptr));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);
}

TEST_F(IlmInputTest, ilm_getInputDevices_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failure
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputDevices, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputDevices(ILM_INPUT_DEVICE_KEYBOARD, &m_numberSeat, &mp_getSeats));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], impl_sync_and_acquire_instance_fake.return_val_history[0]);
}

TEST_F(IlmInputTest, ilm_getInputDevices_success)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputDevices, with right input, expect a success
    ASSERT_EQ(ILM_SUCCESS, ilm_getInputDevices(ILM_INPUT_DEVICE_KEYBOARD, &m_numberSeat, &mp_getSeats));
    // expect result same with prepare data and release_instance should trigger once time
    EXPECT_EQ(1, release_instance_fake.call_count);
    EXPECT_EQ(5, m_numberSeat);
    for(uint8_t i = 0; i < m_numberSeat; i++)
    {
        EXPECT_EQ(0, strcmp(mp_getSeats[i], mp_seatName[m_numberSeat - i - 1]));
    }
    // free resource
    for(uint8_t i = 0; i < m_numberSeat; i++)
    {
        free(mp_getSeats[i]);
    }
    free(mp_getSeats);
}

TEST_F(IlmInputTest, ilm_getInputDeviceCapabilities_invalidAgrument)
{
    // Invoke the ilm_getInputDeviceCapabilities with wrong input, expect a failure
    ilmInputDevice bitmask;
    t_ilm_string l_stringSeat = (t_ilm_string)"seat_1";
    ASSERT_EQ(ILM_FAILED, ilm_getInputDeviceCapabilities(nullptr, &bitmask));
    ASSERT_EQ(ILM_FAILED, ilm_getInputDeviceCapabilities(l_stringSeat, nullptr));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);

}

TEST_F(IlmInputTest, ilm_getInputDeviceCapabilities_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failure
    ilmInputDevice bitmask;
    t_ilm_string l_stringSeat = (t_ilm_string)"seat_1";
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputDeviceCapabilities, expect a failure
    ASSERT_EQ(ILM_FAILED, ilm_getInputDeviceCapabilities(l_stringSeat, &bitmask));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], impl_sync_and_acquire_instance_fake.return_val_history[0]);
}

TEST_F(IlmInputTest, ilm_getInputDeviceCapabilities_success)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    ilmInputDevice bitmask;
    t_ilm_string l_stringSeat = (t_ilm_string)"seat_1";
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputDeviceCapabilities, with right input, expect a success
    ASSERT_EQ(ILM_SUCCESS, ilm_getInputDeviceCapabilities(l_stringSeat, &bitmask));
    // The bitmask should make ILM_INPUT_DEVICE_ALL, the release_instance should trigger once time
    ASSERT_EQ(ILM_INPUT_DEVICE_ALL, bitmask);
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputFocus_invalidAgrument)
{
    // Invoke the ilm_setInputFocus with wrong input, expect the failure
    t_ilm_surface l_surfaceIDs[] = {MAX_NUMBER + 1, MAX_NUMBER + 2};
    ASSERT_EQ(ILM_FAILED, ilm_setInputFocus(nullptr, 2, ILM_INPUT_DEVICE_POINTER | ILM_INPUT_DEVICE_KEYBOARD, ILM_TRUE));
    ASSERT_EQ(ILM_FAILED, ilm_setInputFocus(l_surfaceIDs, 2, ILM_INPUT_DEVICE_POINTER | ILM_INPUT_DEVICE_KEYBOARD, ILM_TRUE));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);
}

TEST_F(IlmInputTest, ilm_setInputFocus_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failure
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke to ilm_setInputFocus, expect a failure
    t_ilm_surface l_surfaceIDs[] = {1, 2};
    ASSERT_EQ(ILM_FAILED, ilm_setInputFocus(l_surfaceIDs, 2, ILM_INPUT_DEVICE_KEYBOARD, ILM_TRUE));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], impl_sync_and_acquire_instance_fake.return_val_history[0]);
}

TEST_F(IlmInputTest, ilm_setInputFocus_surfaceNotFound)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_setInputFocus with wrong input surface, expect the failure
    t_ilm_surface l_surfaceIDs[] = {MAX_NUMBER + 1, MAX_NUMBER + 2};
    ASSERT_EQ(ILM_FAILED, ilm_setInputFocus(l_surfaceIDs, 2, ILM_INPUT_DEVICE_KEYBOARD, ILM_TRUE));
    // the release_instance should trigger once time
    ASSERT_EQ(1, release_instance_fake.call_count);
}

TEST_F(IlmInputTest, ilm_setInputFocus_success)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_setInputFocus with right input, expect the success
    t_ilm_surface l_surfaceIDs[] = {1};
    ASSERT_EQ(ILM_SUCCESS, ilm_setInputFocus(l_surfaceIDs, 1, ILM_INPUT_DEVICE_POINTER|ILM_INPUT_DEVICE_KEYBOARD, ILM_TRUE));
    // the wl_proxy_marshal_flags and release_instance should trigger once time
    ASSERT_EQ(1, release_instance_fake.call_count);
    ASSERT_EQ(1, wl_proxy_marshal_flags_fake.call_count);
}

TEST_F(IlmInputTest, ilm_getInputFocus_invalidAgrument)
{
    // Invoke the ilm_getInputFocus with wrong input, expect the failure
    t_ilm_surface *lp_surfaceIds = nullptr;
    ilmInputDevice *lp_bitmasks = nullptr;
    t_ilm_uint l_numId = 0;
    ASSERT_EQ(ILM_FAILED, ilm_getInputFocus(nullptr, &lp_bitmasks, &l_numId));
    ASSERT_EQ(ILM_FAILED, ilm_getInputFocus(&lp_surfaceIds, nullptr, &l_numId));
    ASSERT_EQ(ILM_FAILED, ilm_getInputFocus(&lp_surfaceIds, &lp_bitmasks, nullptr));
    // impl_sync_and_acquire_instance should not trigger
    ASSERT_EQ(impl_sync_and_acquire_instance_fake.call_count, 0);
}

TEST_F(IlmInputTest, ilm_getInputFocus_cannotSyncAcquireInstance)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return failure
    mp_ilmErrorType[0] = ILM_FAILED;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke ilm_getInputFocus, expect the failure
    t_ilm_surface *lp_surfaceIds = nullptr;
    ilmInputDevice *lp_bitmasks = nullptr;
    t_ilm_uint l_numId = 0;
    ASSERT_EQ(ILM_FAILED, ilm_getInputFocus(&lp_surfaceIds, &lp_bitmasks, &l_numId));
    // impl_sync_and_acquire_instance should trigger once time
    ASSERT_EQ(1, impl_sync_and_acquire_instance_fake.call_count);
    ASSERT_EQ(mp_ilmErrorType[0], impl_sync_and_acquire_instance_fake.return_val_history[0]);
}

TEST_F(IlmInputTest, ilm_getInputFocus_success)
{
    // Prepare fake for impl_sync_and_acquire_instance, to return success
    mp_ilmErrorType[0] = ILM_SUCCESS;
    SET_RETURN_SEQ(impl_sync_and_acquire_instance, mp_ilmErrorType, 1);
    // Invoke the ilm_getInputFocus with right input, expect the success
    t_ilm_surface *lp_surfaceIds = nullptr;
    ilmInputDevice *lp_bitmasks = nullptr;
    t_ilm_uint l_numId = 0;
    ASSERT_EQ(ILM_SUCCESS, ilm_getInputFocus(&lp_surfaceIds, &lp_bitmasks, &l_numId));
    // return data should same with prepare data
    EXPECT_EQ(1, release_instance_fake.call_count);
    EXPECT_EQ(5, l_numId);
    for(uint8_t i = 0; i < l_numId; i++)
    {
        EXPECT_EQ(lp_surfaceIds[i], mp_ilmSurfaceIds[l_numId - i -1]);
        EXPECT_EQ(lp_bitmasks[i], ILM_INPUT_DEVICE_KEYBOARD);
    }
    //free resource
    free(lp_surfaceIds);
    free(lp_bitmasks);
}

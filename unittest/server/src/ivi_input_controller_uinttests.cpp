#include "ivi_input_controller_base_class.hpp"
#include "ivi-controller.h"
#include "ivi-input-server-protocol.h"
#include <gtest/gtest.h>
#include "ivi_layout_structure.hpp"

static constexpr uint8_t MAX_NUMBER = 2;
static constexpr uint8_t DEFAULT_SEAT = 0;
static constexpr uint8_t CUSTOM_SEAT = 1;

class InputControllerTest: public ::testing::Test, public InputControllerBase
{
public:
    void SetUp()
    {
        ASSERT_EQ(initBaseModule(), true);
        IVI_LAYOUT_FAKE_LIST(RESET_FAKE);
        SERVER_API_FAKE_LIST(RESET_FAKE);
        init_input_controller_content();
    }

    void TearDown()
    {
        deinit_input_controller_content();
    }

    void init_input_controller_content()
    {
        // Prepare sample for ivi shell
        m_iviShell.interface = &g_iviLayoutInterfaceFake;
        m_iviShell.compositor = &m_westonCompositor;
        custom_wl_list_init(&m_iviShell.list_surface);
        custom_wl_list_init(&m_westonCompositor.seat_list);
        // Prepare the input context
        mp_ctxInput = (struct input_context*)calloc(1, sizeof(struct input_context));
        mp_ctxInput->ivishell = &m_iviShell;
        custom_wl_list_init(&mp_ctxInput->seat_list);
        custom_wl_list_init(&mp_ctxInput->resource_list);

        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            //Prepare for weston seats
            mp_westonSeat[i].seat_name = (char*)mp_seatName[i];
            custom_wl_list_init(&mp_westonSeat[i].destroy_signal.listener_list);
            custom_wl_list_init(&mp_westonSeat[i].updated_caps_signal.listener_list);
            //Prepare for seat context
            mpp_ctxSeat[i] = (struct seat_ctx*)calloc(1, sizeof(struct seat_ctx));
            mpp_ctxSeat[i]->input_ctx = mp_ctxInput;
            mpp_ctxSeat[i]->west_seat = &mp_westonSeat[i];
            custom_wl_list_insert(&mp_ctxInput->seat_list, &mpp_ctxSeat[i]->seat_node);
            //Prepare for resource
            mp_wlResource[i].object.id = i + 1;
            custom_wl_list_insert(&mp_ctxInput->resource_list, &mp_wlResource[i].link);
            //Prepare for surface
            mp_layoutSurface[i].id_surface = i + 10;
            mp_iviSurface[i].shell = &m_iviShell;
            mp_iviSurface[i].layout_surface = &mp_layoutSurface[i];
            custom_wl_list_init(&mp_iviSurface[i].accepted_seat_list);
            custom_wl_list_insert(&m_iviShell.list_surface, &mp_iviSurface[i].link);
            // Prepare for accepted
            mpp_seatFocus[i] = (struct seat_focus*)calloc(1, sizeof(struct seat_focus));
            mpp_seatFocus[i]->seat_ctx =  mpp_ctxSeat[i];
            custom_wl_list_insert(&mp_iviSurface[i].accepted_seat_list, &mpp_seatFocus[i]->link);
        }
    }

    void enable_utility_funcs_of_array_list()
    {
        wl_list_init_fake.custom_fake = custom_wl_list_init;
        wl_list_insert_fake.custom_fake = custom_wl_list_insert;
        wl_list_remove_fake.custom_fake = custom_wl_list_remove;
        wl_list_empty_fake.custom_fake = custom_wl_list_empty;
        wl_array_init_fake.custom_fake = custom_wl_array_init;
        wl_array_release_fake.custom_fake = custom_wl_array_release;
        wl_array_add_fake.custom_fake = custom_wl_array_add;
        wl_signal_init(&m_iviShell.ivisurface_created_signal);
        wl_signal_init(&m_iviShell.ivisurface_removed_signal);
        wl_signal_init(&m_iviShell.compositor->destroy_signal);
        wl_signal_init(&m_iviShell.compositor->seat_created_signal);
    }

    void deinit_input_controller_content()
    {
        if(mp_ctxInput != nullptr)
        {
            free(mp_ctxInput);
        }

        for(uint8_t i = 0; i< MAX_NUMBER; i++)
        {
            if(mpp_ctxSeat[i] != nullptr)
            {
                free(mpp_ctxSeat[i]);
            }
            if(mpp_seatFocus[i] != nullptr)
            {
                free(mpp_seatFocus[i]);
            }
        }
    }

    struct input_context *mp_ctxInput = nullptr;
    struct wl_resource mp_wlResource[MAX_NUMBER];

    struct seat_ctx *mpp_ctxSeat[MAX_NUMBER] = {nullptr};
    struct weston_seat mp_westonSeat[MAX_NUMBER];
    const char* mp_seatName[MAX_NUMBER] = {"default", "weston_seat_1"};

    struct seat_focus *mpp_seatFocus[MAX_NUMBER] = {nullptr};
    struct ivisurface mp_iviSurface[MAX_NUMBER];
    struct ivi_layout_surface mp_layoutSurface[MAX_NUMBER];

    struct weston_compositor m_westonCompositor = {};
    struct ivishell m_iviShell = {};
}; //InputControllerTest

TEST_F(InputControllerTest, handle_seat_create_customSeat)
{
    // Invoke the handle_seat_create with the custom seat
    handle_seat_create(&mp_ctxInput->seat_create_listener, &mp_westonSeat[CUSTOM_SEAT]);
    /* Check the logic when new custom seate create
     * The new seat need to add to the seat list of input context
     * Register for destroy and update caps events
     * All client need to receive the seat create events
     */
    EXPECT_EQ(wl_list_insert_fake.call_count, 3);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, MAX_NUMBER);
    struct seat_ctx *lp_ctxSeat = (struct seat_ctx *)((uintptr_t)wl_list_insert_fake.arg1_history[0] - offsetof(struct seat_ctx, seat_node));
    // free resource
    free(lp_ctxSeat);
}

TEST_F(InputControllerTest, handle_seat_create_defaultSeat)
{
    // Invoke the handle_seat_create with the default seat
    handle_seat_create(&mp_ctxInput->seat_create_listener, &mp_westonSeat[DEFAULT_SEAT]);
    /* Check the logic when new default seate create
     * The new seat need to add to the seat list of input context
     * Register for destroy and update caps events
     * All client need to receive the seat create events
     * All surface need to add new seat to accept list
     * All client will receive the input acceptance events of each surface
     */
    EXPECT_EQ(wl_list_insert_fake.call_count, 5);
    ASSERT_EQ(MAX_NUMBER + MAX_NUMBER*MAX_NUMBER, wl_resource_post_event_fake.call_count);
    struct seat_ctx *lp_ctxSeat = (struct seat_ctx *)((uintptr_t)wl_list_insert_fake.arg1_history[0] - offsetof(struct seat_ctx, seat_node));
    // free resource
    for(uint8_t i = 0; i < MAX_NUMBER; i++)
    {
        struct seat_focus *lp_seatFocus = (struct seat_focus *)((uintptr_t)wl_list_insert_fake.arg1_history[i+3] - offsetof(struct seat_focus, link));
        free(lp_seatFocus);
    }
    free(lp_ctxSeat);
}

TEST_F(InputControllerTest, handle_seat_updated_caps_allAvailable)
{
    // Prepare fake for the weston_seat_get_keyboard, to make return different nullptr
    // Prepare fake for the weston_seat_get_pointer, to make return different nullptr
    // Prepare fake for the weston_seat_get_touch, to make return different nullptr
    struct weston_keyboard *lpp_keyboard [] = {(struct weston_keyboard *)0xFFFFFFFF};
    struct weston_pointer *lpp_pointer [] = {(struct weston_pointer *)0xFFFFFFFF};
    struct weston_touch *lpp_touch [] = {(struct weston_touch *)0xFFFFFFFF};
    SET_RETURN_SEQ(weston_seat_get_keyboard, lpp_keyboard, 1);
    SET_RETURN_SEQ(weston_seat_get_pointer, lpp_pointer, 1);
    SET_RETURN_SEQ(weston_seat_get_touch, lpp_touch, 1);
    // Reset the keyboard_grab, pointer_grab and touch_grab of custom seat
    mpp_ctxSeat[CUSTOM_SEAT]->keyboard_grab.keyboard = nullptr;
    mpp_ctxSeat[CUSTOM_SEAT]->pointer_grab.pointer = nullptr;
    mpp_ctxSeat[CUSTOM_SEAT]->touch_grab.touch = nullptr;
    // Invoke the handle_seat_updated_caps
    handle_seat_updated_caps(&mpp_ctxSeat[CUSTOM_SEAT]->updated_caps_listener, &mp_westonSeat[CUSTOM_SEAT]);
    /* Check the logic when all input avaiable
     * weston_keyboard_start_grab, weston_pointer_start_grab and weston_touch_start_grab need to trigger one time
     * All client will receive the seat_capabilities events
     */
    ASSERT_EQ(1, weston_keyboard_start_grab_fake.call_count);
    ASSERT_EQ(1, weston_pointer_start_grab_fake.call_count);
    ASSERT_EQ(1, weston_touch_start_grab_fake.call_count);
    ASSERT_EQ(MAX_NUMBER, wl_resource_post_event_fake.call_count);
}

TEST_F(InputControllerTest, handle_seat_updated_caps_notAvailable)
{
    // Prepare fake for the weston_seat_get_keyboard, to make return nullptr
    // Prepare fake for the weston_seat_get_pointer, to make return nullptr
    // Prepare fake for the weston_seat_get_touch, to make return nullptr
    struct weston_keyboard *lpp_keyboard [] = {nullptr};
    struct weston_pointer *lpp_pointer [] = {nullptr};
    struct weston_touch *lpp_touch [] = {nullptr};
    SET_RETURN_SEQ(weston_seat_get_keyboard, lpp_keyboard, 1);
    SET_RETURN_SEQ(weston_seat_get_pointer, lpp_pointer, 1);
    SET_RETURN_SEQ(weston_seat_get_touch, lpp_touch, 1);
    // Set keyboard_grab, pointer_grab and touch_grab for custom seat
    mpp_ctxSeat[CUSTOM_SEAT]->keyboard_grab.keyboard = (struct weston_keyboard *)0xFFFFFFFF;
    mpp_ctxSeat[CUSTOM_SEAT]->pointer_grab.pointer = (struct weston_pointer *)0xFFFFFFFF;
    mpp_ctxSeat[CUSTOM_SEAT]->touch_grab.touch = (struct weston_touch *)0xFFFFFFFF;
    // Invoke the handle_seat_updated_caps
    handle_seat_updated_caps(&mpp_ctxSeat[CUSTOM_SEAT]->updated_caps_listener, &mp_westonSeat[CUSTOM_SEAT]);
    /* Check the logic when all input don't avaiable
     * weston_keyboard_start_grab, weston_pointer_start_grab and weston_touch_start_grab need to trigger zero time
     * All client will receive the seat_capabilities events
     */
    ASSERT_EQ(0, weston_keyboard_start_grab_fake.call_count);
    ASSERT_EQ(0, weston_pointer_start_grab_fake.call_count);
    ASSERT_EQ(0, weston_touch_start_grab_fake.call_count);
    ASSERT_EQ(nullptr, mpp_ctxSeat[CUSTOM_SEAT]->keyboard_grab.keyboard);
    ASSERT_EQ(nullptr, mpp_ctxSeat[CUSTOM_SEAT]->pointer_grab.pointer);
    ASSERT_EQ(nullptr, mpp_ctxSeat[CUSTOM_SEAT]->touch_grab.touch);
    ASSERT_EQ(MAX_NUMBER, wl_resource_post_event_fake.call_count);
}

TEST_F(InputControllerTest, handle_seat_destroy_notSurfaceAccepted)
{
    // Prepare the data input, the seat_ctx object
    struct seat_ctx* lp_ctxSeat = (struct seat_ctx*)calloc(1, sizeof(struct seat_ctx));
    lp_ctxSeat->input_ctx = mp_ctxInput;
    lp_ctxSeat->west_seat = &mp_westonSeat[CUSTOM_SEAT];
    custom_wl_list_insert(&mp_ctxInput->seat_list, &lp_ctxSeat->seat_node);
    // Invoke the handle_seat_destroy
    handle_seat_destroy(&lp_ctxSeat->destroy_listener, &mp_westonSeat[CUSTOM_SEAT]);
    /* Check the logic when destroy a seat without any surface accepted
     * All client will receive the seat_destroy events
     * The destroy and update caps event list should remove this seat
     * The seat list should remove this seat
     */
    ASSERT_EQ(MAX_NUMBER, wl_resource_post_event_fake.call_count);
    ASSERT_EQ(3, wl_list_remove_fake.call_count);
}

TEST_F(InputControllerTest, handle_seat_destroy_withSurfaceAccepted)
{
    // Invoke the handle_seat_destroy
    handle_seat_destroy(&mpp_ctxSeat[CUSTOM_SEAT]->destroy_listener, &mp_westonSeat[CUSTOM_SEAT]);
    /* Check the logic when destroy a seat without any surface accepted
     * All client will receive the seat_destroy events
     * The destroy and update caps event list should remove this seat
     * The seat list should remove this seat
     * The accept list of surface should remove this seat, in case is one surface focus
     */
    ASSERT_EQ(MAX_NUMBER, wl_resource_post_event_fake.call_count);
    ASSERT_EQ(4, wl_list_remove_fake.call_count);
    //for logic of test, set null to object to avoid double free
    mpp_ctxSeat[CUSTOM_SEAT] = nullptr;
    mpp_seatFocus[CUSTOM_SEAT] = nullptr;
}

TEST_F(InputControllerTest, input_controller_module_init_WrongInput)
{
    // Prepare the wrong input, there isn't interface in ivishell object
    m_iviShell.interface = nullptr;
    // Invoke the input_controller_module_init, expected return a failure
    ASSERT_NE(input_controller_module_init(&m_iviShell), 0);
}

TEST_F(InputControllerTest, input_controller_module_init_cannotCreateIviInput)
{
    enable_utility_funcs_of_array_list();
    // Don't prepare the wl_global_create, it will return nullptr
    // Invoke the input_controller_module_init, expected return a failure
    ASSERT_NE(input_controller_module_init(&m_iviShell), 0);
    // The wl_global_create should trigger once time
    EXPECT_EQ(wl_global_create_fake.call_count, 1);
    // seat focus had free, need to set it to null, avoid double free
    for(uint8_t i = 0; i< MAX_NUMBER; i++)
    {
        mpp_seatFocus[i] = nullptr;
    }
}

TEST_F(InputControllerTest, input_controller_module_init_canInitSuccess)
{
    // Prepare fake for the wl_global_create, make it return a different null pointer
    enable_utility_funcs_of_array_list();
    struct wl_global *lp_wlGlobal = (struct wl_global*) 0xFFFFFFFF;
    SET_RETURN_SEQ(wl_global_create, &lp_wlGlobal, 1);
    // Invoke the input_controller_module_init, expected return a success
    ASSERT_EQ(input_controller_module_init(&m_iviShell), 0);
    // The wl_global_create should trigger once time
    ASSERT_EQ(wl_global_create_fake.call_count, 1);
    // Check the input controller module member
    struct input_context *lp_ctxInput = (struct input_context *)((uintptr_t)wl_list_init_fake.arg0_history[4] - offsetof(struct input_context, resource_list));
    EXPECT_EQ(lp_ctxInput->ivishell, &m_iviShell);
    EXPECT_NE(lp_ctxInput->surface_created.notify, nullptr);
    EXPECT_NE(lp_ctxInput->surface_destroyed.notify, nullptr);
    EXPECT_NE(lp_ctxInput->compositor_destroy_listener.notify, nullptr);
    EXPECT_NE(lp_ctxInput->seat_create_listener.notify, nullptr);
    EXPECT_EQ(lp_ctxInput->successful_init_stage, 1);
    // free resource
    free(lp_ctxInput);
}


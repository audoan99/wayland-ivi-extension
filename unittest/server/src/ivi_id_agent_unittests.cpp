#include "ivi_id_agent_base_class.hpp"
#include <gtest/gtest.h>

#define INVALID_ID 0xFFFFFFFF
static constexpr uint8_t MAX_NUMBER = 2;

class IdAgentTest: public ::testing::Test, public IdAgentBase
{
public:
    void SetUp()
    {
        ASSERT_EQ(initBaseModule(), true);
        IVI_LAYOUT_FAKE_LIST(RESET_FAKE);
        SERVER_API_FAKE_LIST(RESET_FAKE);
        init_controller_content();
    }

    void TearDown()
    {
        deinit_controller_content();
    }

    void init_controller_content()
    {

        mp_iviIdAgent = (struct ivi_id_agent*)malloc(sizeof(struct ivi_id_agent));
        mp_iviIdAgent->compositor = &m_westonCompositor;
        mp_iviIdAgent->interface = &g_iviLayoutInterfaceFake;
        mp_iviIdAgent->default_surface_id = 100;
        mp_iviIdAgent->default_surface_id_max = 200;
        mp_iviIdAgent->default_behavior_set = 0;

        custom_wl_list_init(&mp_iviIdAgent->app_list);
        custom_wl_list_init(&m_westonCompositor.destroy_signal.listener_list);

        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            // prepare for desktop apps
            mp_dbElem[i] = (struct db_elem*)malloc(sizeof(struct db_elem));
            mp_dbElem[i]->surface_id = 10 + i;
            mp_dbElem[i]->cfg_app_id = (char*)malloc(5);
            mp_dbElem[i]->cfg_title = (char*)malloc(10);
            snprintf(mp_dbElem[i]->cfg_app_id, 5, "%d", i);
            snprintf(mp_dbElem[i]->cfg_title, 10, "idtest%d", i);
            custom_wl_list_insert(&mp_iviIdAgent->app_list, &mp_dbElem[i]->link);
        }
    }

    void deinit_controller_content()
    {
        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            if(mp_dbElem[i] != nullptr)
            {
                free(mp_dbElem[i]->cfg_app_id);
                free(mp_dbElem[i]->cfg_title);
                free(mp_dbElem[i]);
            }
        }
        if(mp_iviIdAgent != nullptr)
        {
            free(mp_iviIdAgent);
        }
    }

    struct ivi_id_agent *mp_iviIdAgent = nullptr;
    struct weston_compositor m_westonCompositor = {};

    struct db_elem *mp_dbElem[MAX_NUMBER] = {nullptr};
    void *mp_fakePointer = (void*)0xFFFFFFFF;
    void *mp_nullPointer = nullptr;

    static uint32_t ms_surfaceId;
    static uint32_t ms_defaultSurfaceId;
    static uint32_t ms_defaultSurfaceIdMax;
    static char *ms_appId;
    static char *ms_appTitle;

};

char *IdAgentTest::ms_appId = (char*)"0";
char *IdAgentTest::ms_appTitle = (char*)"app_1";
uint32_t IdAgentTest::ms_surfaceId = 10;
uint32_t IdAgentTest::ms_defaultSurfaceId = 100;
uint32_t IdAgentTest::ms_defaultSurfaceIdMax = 200;

static int custom_weston_config_next_section_1(struct weston_config *config, struct weston_config_section **section, const char **name)
{
    *name = "desktop-app";
    return 1;
}

static int custom_weston_config_next_section_2(struct weston_config *config, struct weston_config_section **section, const char **name)
{
    return 0;
}

static int custom_weston_config_section_get_uint(struct weston_config_section *section, const char *key, uint32_t *value, uint32_t default_value)
{
    if(strcmp(key, "surface-id") == 0)
    {
        *value = IdAgentTest::ms_surfaceId;
    }
    else if(strcmp(key, "default-surface-id") == 0)
    {
        *value = IdAgentTest::ms_defaultSurfaceId;
    }
    else if(strcmp(key, "default-surface-id-max") == 0)
    {
        *value = IdAgentTest::ms_defaultSurfaceIdMax;
    }
    return 0;
}

static int custom_weston_config_section_get_string(struct weston_config_section *section, const char *key, char **value, const char *default_value)
{
    if(strcmp(key, "app-id") == 0)
    {
        *value = (IdAgentTest::ms_appId != nullptr) ? strdup(IdAgentTest::ms_appId) : nullptr;
    }
    else if(strcmp(key, "app-title") == 0)
    {
        *value = (IdAgentTest::ms_appTitle != nullptr) ? strdup(IdAgentTest::ms_appId) : nullptr;
    }
    return 0;
}

TEST_F(IdAgentTest, desktop_surface_event_configure_hasDataInList)
{
    // Prepare fake for weston_desktop_surface_get_app_id and weston_desktop_surface_get_title
    SET_RETURN_SEQ(weston_desktop_surface_get_app_id, (const char**)&mp_dbElem[0]->cfg_app_id, 1);
    SET_RETURN_SEQ(weston_desktop_surface_get_title, (const char**)&mp_dbElem[0]->cfg_title, 1);
    // Invoke the send_surface_prop
    desktop_surface_event_configure(&mp_iviIdAgent->desktop_surface_configured, mp_fakePointer);
    /* Check the logic
     * if new desktop surface have same appId and appTitle in dbElem list, the lib should be add 
     * surface id to the new desktop surface
     */
    ASSERT_EQ(surface_set_id_fake.call_count, 1);
    ASSERT_EQ(mp_dbElem[0]->layout_surface, mp_fakePointer);
    ASSERT_EQ(surface_set_id_fake.arg1_history[0], mp_dbElem[0]->surface_id);
}

TEST_F(IdAgentTest, desktop_surface_event_configure_noDataInListNoDefaultBehavior)
{
    // Prepare fake for weston_desktop_surface_get_app_id and weston_desktop_surface_get_title
    SET_RETURN_SEQ(weston_desktop_surface_get_app_id, (const char**)&mp_dbElem[1]->cfg_app_id, 1);
    SET_RETURN_SEQ(weston_desktop_surface_get_title, (const char**)&mp_dbElem[0]->cfg_title, 1);
    // Invoke the send_surface_prop
    desktop_surface_event_configure(&mp_iviIdAgent->desktop_surface_configured, mp_fakePointer);
    /* Check the logic
     * if new desktop surface don't same appId or appTitle and the default behavior is disable.
     * the surface_set_id should not trigger
     */
    ASSERT_EQ(surface_set_id_fake.call_count, 0);
}

TEST_F(IdAgentTest, desktop_surface_event_configure_noDataInListHasDefaultBehavior)
{
    // enable the default behavior
    // Prepare fake for weston_desktop_surface_get_app_id and weston_desktop_surface_get_title
    mp_iviIdAgent->default_behavior_set = 1;
    SET_RETURN_SEQ(weston_desktop_surface_get_app_id, (const char**)&mp_dbElem[1]->cfg_app_id, 1);
    SET_RETURN_SEQ(weston_desktop_surface_get_title, (const char**)&mp_dbElem[0]->cfg_title, 1);
    // Invoke the send_surface_prop
    desktop_surface_event_configure(&mp_iviIdAgent->desktop_surface_configured, mp_fakePointer);
    /* Check the logic
     * if new desktop surface don't same appId or appTitle, and the default behavior is enable.
     * The lib will tried to assign default id to new desktop surface
     * The surface_set_id_fake must trigger
     */
    ASSERT_EQ(surface_set_id_fake.call_count, 1);
    ASSERT_EQ(surface_set_id_fake.arg1_history[0], mp_iviIdAgent->default_surface_id - 1);
}

TEST_F(IdAgentTest, desktop_surface_event_configure_noDataInListHasDefaultBehaviorExistSurfaceId)
{
    // enable the default behavior
    // Prepare fake for weston_desktop_surface_get_app_id and weston_desktop_surface_get_title
    mp_iviIdAgent->default_behavior_set = 1;
    SET_RETURN_SEQ(weston_desktop_surface_get_app_id, (const char**)&mp_dbElem[1]->cfg_app_id, 1);
    SET_RETURN_SEQ(weston_desktop_surface_get_title, (const char**)&mp_dbElem[0]->cfg_title, 1);
    // Prepare fake for get_surface_from_id return a fake pointer not nullptr
    struct ivi_layout_surface *lp_fakeLayoutLayer = (struct ivi_layout_surface *)0xFFFFFF00;
    SET_RETURN_SEQ(get_surface_from_id, &lp_fakeLayoutLayer, 1);
    // Invoke the send_surface_prop
    desktop_surface_event_configure(&mp_iviIdAgent->desktop_surface_configured, mp_fakePointer);
    /* Check the logic
     * if new desktop surface don't same appId or appTitle and the default behavior is enable
     * The lib will tried to assign default id to new desktop surface, if there is a surface using the default id, it should faile to assign
     * The surface_set_id_fake should not trigger
     */
    ASSERT_EQ(surface_set_id_fake.call_count, 0);
}

TEST_F(IdAgentTest, desktop_surface_event_configure_noDataInListHasDefaultBehaviorOverMaxId)
{
    // enable the default behavior, setup the default id and default max id are same
    // Prepare fake for weston_desktop_surface_get_app_id and weston_desktop_surface_get_title
    mp_iviIdAgent->default_behavior_set = 1;
    mp_iviIdAgent->default_surface_id = 200;
    mp_iviIdAgent->default_surface_id_max = 200;
    SET_RETURN_SEQ(weston_desktop_surface_get_app_id, (const char**)&mp_dbElem[1]->cfg_app_id, 1);
    SET_RETURN_SEQ(weston_desktop_surface_get_title, (const char**)&mp_dbElem[0]->cfg_title, 1);
    // Invoke the send_surface_prop
    desktop_surface_event_configure(&mp_iviIdAgent->desktop_surface_configured, mp_fakePointer);
    /* Check the logic
     * if new desktop surface don't same appId or appTitle, and the default behavior is enable
     * The lib will tried to assign default id to new desktop surface, if the default id is equal or bigger default max id, it should faile to assign
     * The surface_set_id should not trigger
     */
    ASSERT_EQ(surface_set_id_fake.call_count, 0);
}

TEST_F(IdAgentTest, surface_event_remove)
{
    // Set the layout surface for db elem, to check function
    mp_dbElem[0]->layout_surface = (struct ivi_layout_surface *)mp_fakePointer;
    mp_dbElem[1]->layout_surface = (struct ivi_layout_surface *)0xFFFFFF00;
    // Invoke the surface_event_remove
    surface_event_remove(&mp_iviIdAgent->surface_removed, mp_fakePointer);
    /* Check the logic
     * if the db elem have the layout surface is mp_fakePointer, it should reset to null. if not, keep the right pointer
     */
    ASSERT_EQ(mp_dbElem[0]->layout_surface, nullptr);
    ASSERT_EQ(mp_dbElem[1]->layout_surface, (struct ivi_layout_surface *)0xFFFFFF00);
}

TEST_F(IdAgentTest, id_agent_module_deinit)
{
    // Invoke the id_agent_module_deinit
    id_agent_module_deinit(&mp_iviIdAgent->destroy_listener, nullptr);
    /* Check the logic
     * When remove, the wl_list_remove need to trigger to remove the desktop_surface_configured, destroy_listener and surface_removed events
     * free all allocations, the asan will check it.
     */
    ASSERT_EQ(wl_list_remove_fake.call_count, 5);
    // for logic tests
    for(uint8_t i = 0; i < MAX_NUMBER; i++)
    {
        mp_dbElem[i] = nullptr;
    }
    mp_iviIdAgent = nullptr;
}

TEST_F(IdAgentTest, id_agent_module_init_cannotGetwestonConfig)
{
    // Prepare fake of the wl_list custom functions
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Defaut the wet_get_config return nullptr, don't need the fake
    // Invoke the id_agent_module_init, expected it return a failure
    ASSERT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), IVI_FAILED);
    /* Check the logic
     * If the weston config can't get, the id_agent_module_init should return a failure
     * The weston_config_get_section_fake should not trigger
     */
    ASSERT_EQ(wet_get_config_fake.call_count, 1);
    ASSERT_EQ(weston_config_get_section_fake.call_count, 0);
}

TEST_F(IdAgentTest, id_agent_module_init_noDefaultBehaviorNoDesktopApp)
{
    // Prepare fake of the wl_list custom functions
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config, to make return a fake pointer different nullptr
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    // Invoke the id_agent_module_init, expected it return a failure
    ASSERT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), IVI_FAILED);
    /* Check the logic
     * If no desktop-app and no default behavior, the section get integer and string should not trigger
     * The wl_list_empty_fake should trigger and return 0
     */
    ASSERT_EQ(wet_get_config_fake.call_count, 1);
    ASSERT_EQ(weston_config_get_section_fake.call_count, 1);
    ASSERT_EQ(weston_config_next_section_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_uint_fake.call_count, 0);
    ASSERT_EQ(weston_config_section_get_string_fake.call_count, 0);
    ASSERT_NE(wl_list_empty_fake.return_val_history[0], 0);
}

TEST_F(IdAgentTest, id_agent_module_init_hasDefaultBehaviorNoDesktopApp)
{
    // Prepare fake of the wl_list custom functions
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config and weston_config_get_section, to make sure return different null
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    SET_RETURN_SEQ(weston_config_get_section, (struct weston_config_section **)&mp_fakePointer, 1);
    // Prepare fake for weston_config_section_get_uint
    IdAgentTest::ms_defaultSurfaceId = 100;
    IdAgentTest::ms_defaultSurfaceIdMax = 200;
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    // Invoke the id_agent_module_init, expected a success
    EXPECT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), IVI_SUCCEEDED);
    /* Check the logic
     * If no desktop-app and default behavior enable, the weston_config_section_get_uint should trigger two times
     */
    EXPECT_EQ(wet_get_config_fake.call_count, 1);
    EXPECT_EQ(weston_config_get_section_fake.call_count, 1);
    EXPECT_EQ(weston_config_section_get_uint_fake.call_count, 2);
    EXPECT_EQ(weston_config_section_get_string_fake.call_count, 0);
    //free resource
    struct ivi_id_agent *lp_idAgent = (struct ivi_id_agent*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivi_id_agent, app_list));
    free(lp_idAgent);
}

// @Todo this have issue with doulbe free, the dbElem->appTitle, dbElem->appId is not allocated, but free when deinit
TEST_F(IdAgentTest, id_agent_module_init_noDefaultBehaviorHasDesktopAppWithInvalidSurfaceId)
{
    // Prepare fake of the wl_list custom function
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config and weston_config_next_section, first time return true and there is a desktop-app section, second time return a failure
    int (*weston_config_next_section_fakes[])(struct weston_config *, struct weston_config_section **, const char **) = {
        custom_weston_config_next_section_1,
        custom_weston_config_next_section_2
    };
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    SET_CUSTOM_FAKE_SEQ(weston_config_next_section, weston_config_next_section_fakes, 2);
    // Prepare return invalid id of surface
    IdAgentTest::ms_surfaceId = INVALID_ID;
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    // Invoke the id_agent_module_init, expected a failure
    ASSERT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), IVI_FAILED);
    /* Check the logic
     * If lib get the invalid of surface id in desktop-app section, it should return a failure
     */
    ASSERT_EQ(wet_get_config_fake.call_count, 1);
    ASSERT_EQ(weston_config_next_section_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_uint_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_string_fake.call_count, 0);
}

// @Todo this have issue with doulbe free, the dbElem->appTitle, dbElem->appId is not allocated, but free when deinit
TEST_F(IdAgentTest, id_agent_module_init_noDefaultBehaviorHasDesktopAppWithNullOfAppIdAndAppTitle)
{
    // Prepare fake of the wl_list custom function
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config and weston_config_next_section, first time return true and there is a desktop-app section, second time return a failure
    int (*weston_config_next_section_fakes[])(struct weston_config *, struct weston_config_section **, const char **) = {
        custom_weston_config_next_section_1,
        custom_weston_config_next_section_2
    };
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    SET_CUSTOM_FAKE_SEQ(weston_config_next_section, weston_config_next_section_fakes, 2);
    // Prepare return valid id of surface and nullptr for appTitle and appId
    IdAgentTest::ms_surfaceId = 10;
    IdAgentTest::ms_appId = nullptr;
    IdAgentTest::ms_appTitle = nullptr;
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    weston_config_section_get_string_fake.custom_fake = custom_weston_config_section_get_string;
    // Invoke the id_agent_module_init, expected return a failure
    ASSERT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), IVI_FAILED);
    /* Check the logic
     * If lib get the nullptr for of app-title and app-id in desktop-app section, it should return a failure
     */
    ASSERT_EQ(wet_get_config_fake.call_count, 1);
    ASSERT_EQ(weston_config_next_section_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_uint_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_string_fake.call_count, 2);
}

// @Todo maybe have logic issue here, do we must have the default-app-default section?
// only that section configed the default id and default max id
TEST_F(IdAgentTest, id_agent_module_init_noDefaultBehaviorHasDesktopAppWithRightConfig)
{
    // Prepare fake of the wl_list custom function
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config and weston_config_next_section, first time return true and there is a desktop-app section, second time return a failure
    int (*weston_config_next_section_fakes[])(struct weston_config *, struct weston_config_section **, const char **) = {
        custom_weston_config_next_section_1,
        custom_weston_config_next_section_2
    };
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    SET_CUSTOM_FAKE_SEQ(weston_config_next_section, weston_config_next_section_fakes, 2);
    // Prepare the right appId, AppTitle and surface id
    IdAgentTest::ms_surfaceId = 0;
    IdAgentTest::ms_appId = (char*)"0";
    IdAgentTest::ms_appTitle = (char*)"app_0";
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    weston_config_section_get_string_fake.custom_fake = custom_weston_config_section_get_string;
    // Invoke the id_agent_module_init, expected a success
    ASSERT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), -1);

    /* Check the logic
     * If lib get the expected string for of app-title and app-id, surface id in desktop-app section, it should return a success
     */
    ASSERT_EQ(wet_get_config_fake.call_count, 1);
    ASSERT_EQ(weston_config_next_section_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_uint_fake.call_count, 1);
    ASSERT_EQ(weston_config_section_get_string_fake.call_count, 2);
    //free resource
    // struct ivi_id_agent *lp_idAgent = (struct ivi_id_agent*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivi_id_agent, app_list));
    // id_agent_module_deinit(&lp_idAgent->destroy_listener, nullptr);
}

// @Todo maybe have logic issue here, range of surface id in check_config functions
// enable free resoure, if there is a change in the lib
TEST_F(IdAgentTest, id_agent_module_init_hasDefaultBehaviorHasDesktopAppWithRightConfig)
{
    // Prepare fake of the wl_list custom function
    wl_list_init_fake.custom_fake = custom_wl_list_init;
    wl_list_insert_fake.custom_fake = custom_wl_list_insert;
    wl_list_empty_fake.custom_fake = custom_wl_list_empty;
    // Prepare fake for wet_get_config, weston_config_get_section and weston_config_next_section, first time return true and there is a desktop-app section, second time return a failure
    int (*weston_config_next_section_fakes[])(struct weston_config *, struct weston_config_section **, const char **) = {
        custom_weston_config_next_section_1,
        custom_weston_config_next_section_2
    };
    SET_RETURN_SEQ(wet_get_config, (struct weston_config **)&mp_fakePointer, 1);
    SET_RETURN_SEQ(weston_config_get_section, (struct weston_config_section **)&mp_fakePointer, 1);
    SET_CUSTOM_FAKE_SEQ(weston_config_next_section, weston_config_next_section_fakes, 2);
    // Prepare the right appId, AppTitle and surface id
    IdAgentTest::ms_defaultSurfaceId = 100;
    IdAgentTest::ms_defaultSurfaceIdMax = 200;
    IdAgentTest::ms_surfaceId = IdAgentTest::ms_defaultSurfaceId + 1;
    IdAgentTest::ms_appId = (char*)"0";
    IdAgentTest::ms_appTitle = (char*)"app_0";
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    weston_config_section_get_string_fake.custom_fake = custom_weston_config_section_get_string;
    // Invoke the id_agent_module_init, expected a success
    EXPECT_EQ(id_agent_module_init(&m_westonCompositor, &g_iviLayoutInterfaceFake), -1);
    /* Check the logic
     * Enable default behavior
     * If lib get the expected string for of app-title and app-id, surface id in desktop-app section, it should return a success
     */
    EXPECT_EQ(wet_get_config_fake.call_count, 1);
    EXPECT_EQ(weston_config_next_section_fake.call_count, 1);
    EXPECT_EQ(weston_config_get_section_fake.call_count, 1);
    EXPECT_EQ(weston_config_section_get_uint_fake.call_count, 3);
    EXPECT_EQ(weston_config_section_get_string_fake.call_count, 2);
    //free resource
    struct ivi_id_agent *lp_idAgent = (struct ivi_id_agent*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivi_id_agent, app_list));
    //id_agent_module_deinit(&lp_idAgent->destroy_listener, nullptr);
}

#include <gtest/gtest.h>
#include "ivi_controller_base_class.hpp"
#include "ivi_layout_structure.hpp"

static constexpr uint8_t MAX_NUMBER = 2;
static uint32_t g_SurfaceCreatedCount = 0;
static uint32_t custom_get_id_of_surface(struct ivi_layout_surface *ivisurf)
{
    return ivisurf->id_surface;
}

static uint32_t custom_get_id_of_layer(struct ivi_layout_layer *ivilayer)
{
    return ivilayer->id_layer;
}

static void surface_create_callback(struct wl_listener *listener, void *data)
{
    g_SurfaceCreatedCount++;
}

class ControllerTests : public ::testing::Test, public ControllerBase
{
public:
    void SetUp()
    {
        ASSERT_EQ(initBaseModule(), true);
        IVI_LAYOUT_FAKE_LIST(RESET_FAKE);
        SERVER_API_FAKE_LIST(RESET_FAKE);
        get_id_of_surface_fake.custom_fake = custom_get_id_of_surface;
        get_id_of_layer_fake.custom_fake = custom_get_id_of_layer;
        g_SurfaceCreatedCount = 0;
        init_controller_content();
    }

    void TearDown()
    {
        deinit_controller_content();
    }

    void init_controller_content()
    {
        uint8_t l_fakeResource = 0, l_fakeClient = 0;
        mp_iviShell = (struct ivishell *)malloc(sizeof(struct ivishell));
        mp_iviShell->interface = &g_iviLayoutInterfaceFake;
        mp_iviShell->compositor = &m_westonCompositor;
        mp_iviShell->bkgnd_surface_id = 1;
        mp_iviShell->bkgnd_surface = nullptr;
        mp_iviShell->bkgnd_view = nullptr;
        mp_iviShell->client = nullptr;
        mp_iviShell->screen_id_offset = 1000;
        custom_wl_array_init(&mp_iviShell->screen_ids);
        mp_screenInfo = (struct screen_id_info *)custom_wl_array_add(&mp_iviShell->screen_ids, sizeof(struct screen_id_info));
        mp_screenInfo->screen_name = (char *)"default_screen";
        mp_screenInfo->screen_id = 0;

        custom_wl_list_init(&m_westonCompositor.output_list);
        custom_wl_list_init(&mp_iviShell->list_surface);
        custom_wl_list_init(&mp_iviShell->list_layer);
        custom_wl_list_init(&mp_iviShell->list_screen);
        custom_wl_list_init(&mp_iviShell->list_controller);
        custom_wl_list_init(&mp_iviShell->ivisurface_created_signal.listener_list);
        custom_wl_list_init(&mp_iviShell->ivisurface_removed_signal.listener_list);

        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            // Prepare layout surfaces prperties
            m_layoutSurface[i].id_surface = i + 10;
            m_layoutSurfaceProperties[i].opacity = 1000 + i*100;
            m_layoutSurfaceProperties[i].source_x = 0;
            m_layoutSurfaceProperties[i].source_y = 0;
            m_layoutSurfaceProperties[i].source_width = 100 + i*10;
            m_layoutSurfaceProperties[i].source_height = 100 + i*10;
            m_layoutSurfaceProperties[i].start_x = 0;
            m_layoutSurfaceProperties[i].start_y = 0;
            m_layoutSurfaceProperties[i].start_width = 50 + i*20;
            m_layoutSurfaceProperties[i].start_height = 50 + i*20;
            m_layoutSurfaceProperties[i].dest_x = 0;
            m_layoutSurfaceProperties[i].dest_y = 0;
            m_layoutSurfaceProperties[i].dest_width = 300 + i*5;
            m_layoutSurfaceProperties[i].dest_height = 300 + i*5;
            m_layoutSurfaceProperties[i].orientation = WL_OUTPUT_TRANSFORM_NORMAL;
            m_layoutSurfaceProperties[i].visibility = true;
            m_layoutSurfaceProperties[i].transition_type = 1;
            m_layoutSurfaceProperties[i].transition_duration = 20;
            m_layoutSurfaceProperties[i].event_mask = 0;

            // Prepare the ivi surface
            mp_iviSurface[i] = (struct ivisurface*)malloc(sizeof(struct ivisurface));
            mp_iviSurface[i]->shell = mp_iviShell;
            mp_iviSurface[i]->layout_surface = &m_layoutSurface[i];
            mp_iviSurface[i]->prop = &m_layoutSurfaceProperties[i];
            custom_wl_list_insert(&mp_iviShell->list_surface, &mp_iviSurface[i]->link);

            // the client callback, it listen the change from specific ivi surface id
            mp_surfaceNotification[i] = (struct notification*)malloc(sizeof(struct notification));
            mp_surfaceNotification[i]->resource = (struct wl_resource*)((uintptr_t)mp_wlResourceDefault + i);
            custom_wl_list_init(&mp_iviSurface[i]->notification_list);
            custom_wl_list_insert(&mp_iviSurface[i]->notification_list, &mp_surfaceNotification[i]->layout_link);

            // Prepare layout layer properties
            m_layoutLayer[i].id_layer = i + 100;
            m_layoutLayerProperties[i].opacity = 1000 + i*100;
            m_layoutLayerProperties[i].source_x = 0;
            m_layoutLayerProperties[i].source_y = 0;
            m_layoutLayerProperties[i].source_width = 100 + i*10;
            m_layoutLayerProperties[i].source_height = 100 + i*10;
            m_layoutLayerProperties[i].dest_x = 0;
            m_layoutLayerProperties[i].dest_y = 0;
            m_layoutLayerProperties[i].dest_width = 300 + i*5;
            m_layoutLayerProperties[i].dest_height = 300 + i*5;
            m_layoutLayerProperties[i].orientation = WL_OUTPUT_TRANSFORM_NORMAL;
            m_layoutLayerProperties[i].visibility = true;
            m_layoutLayerProperties[i].transition_type = 1;
            m_layoutLayerProperties[i].transition_duration = 20;
            m_layoutLayerProperties[i].start_alpha = 0.1;
            m_layoutLayerProperties[i].end_alpha = 10.0;
            m_layoutLayerProperties[i].is_fade_in = 1 ;
            m_layoutLayerProperties[i].event_mask = 0 ;

            // Prepare the ivi layer
            mp_iviLayer[i] = (struct ivilayer *)malloc(sizeof(struct ivilayer));
            mp_iviLayer[i]->shell = mp_iviShell;
            mp_iviLayer[i]->layout_layer = &m_layoutLayer[i];
            mp_iviLayer[i]->prop = &m_layoutLayerProperties[i];
            custom_wl_list_insert(&mp_iviShell->list_layer, &mp_iviLayer[i]->link);

            // The client callback, it lusten the change from specific ivi layer id
            mp_LayerNotification[i] = (struct notification *)malloc(sizeof(struct notification));
            mp_LayerNotification[i]->resource = (struct wl_resource*)((uintptr_t)mp_wlResourceDefault + i);
            custom_wl_list_init(&mp_iviLayer[i]->notification_list);
            custom_wl_list_insert(&mp_iviLayer[i]->notification_list, &mp_LayerNotification[i]->layout_link);

            //Prepare the ivi screens
            m_westonOutput[i].id = i + 1000;
            mp_iviScreen[i] = (struct iviscreen*)malloc(sizeof(struct iviscreen));
            mp_iviScreen[i]->shell = mp_iviShell;
            mp_iviScreen[i]->output = &m_westonOutput[i];
            mp_iviScreen[i]->id_screen = m_westonOutput[i].id;
            custom_wl_list_insert(&mp_iviShell->list_screen, &mp_iviScreen[i]->link);

            // Prepare sample controllers
            mp_iviController[i] = (struct ivicontroller*)malloc(sizeof(struct ivicontroller));
            mp_iviController[i]->resource = (struct wl_resource*)&l_fakeResource;
            mp_iviController[i]->client = (struct wl_client*)&l_fakeClient;
            mp_iviController[i]->shell = mp_iviShell;
            mp_iviController[i]->id = i;
            custom_wl_list_insert(&mp_iviShell->list_controller, &mp_iviController[i]->link);

            // Prepare handler for new surfaces
            m_listenSurface[i].notify = surface_create_callback;
            custom_wl_list_insert(mp_iviShell->ivisurface_created_signal.listener_list.prev, &m_listenSurface[i].link);
        }
    }

    void deinit_controller_content()
    {
        for(uint8_t i = 0; i < MAX_NUMBER; i++)
        {
            free(mp_surfaceNotification[i]);
            free(mp_iviSurface[i]);
            free(mp_LayerNotification[i]);
            free(mp_iviLayer[i]);
            free(mp_iviScreen[i]);
            free(mp_iviController[i]);
        }
        custom_wl_array_release(&mp_iviShell->screen_ids);
        free(mp_iviShell);
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
        wl_signal_init(&m_westonCompositor.output_created_signal);
        wl_signal_init(&m_westonCompositor.output_destroyed_signal);
        wl_signal_init(&m_westonCompositor.output_resized_signal);
        wl_signal_init(&m_westonCompositor.destroy_signal);
    }

    struct wl_resource *mp_wlResourceDefault = (struct wl_resource *)0xFFFFFF00;

    struct wl_listener m_listenSurface[MAX_NUMBER] = {};
    struct ivisurface *mp_iviSurface[MAX_NUMBER] = {nullptr};
    struct ivi_layout_surface m_layoutSurface[MAX_NUMBER] = {};
    struct ivi_layout_surface_properties m_layoutSurfaceProperties[MAX_NUMBER] = {};
    struct notification *mp_surfaceNotification[MAX_NUMBER] = {nullptr};

    struct ivilayer *mp_iviLayer[MAX_NUMBER] = {nullptr};
    struct ivi_layout_layer m_layoutLayer[MAX_NUMBER] = {};
    struct ivi_layout_layer_properties m_layoutLayerProperties[MAX_NUMBER] = {};
    struct notification *mp_LayerNotification[MAX_NUMBER] = {nullptr};

    struct iviscreen *mp_iviScreen[MAX_NUMBER] = {nullptr};
    struct weston_output m_westonOutput[MAX_NUMBER] = {};

    struct ivicontroller *mp_iviController[MAX_NUMBER] = {nullptr};

    struct ivishell *mp_iviShell = nullptr;
    struct weston_compositor m_westonCompositor = {};

    struct screen_id_info *mp_screenInfo = nullptr;

    struct wl_client *mp_fakeClient = nullptr;

    static int ms_idAgentModuleValue;
    static int ms_inputControllerModuleValue;
    static uint32_t ms_screenIdOffset;
    static uint32_t ms_screenId;
    static char *ms_iviClientName;
    static char *ms_debugScopes;
    static char *ms_screenName;
    static int32_t ms_bkgndSurfaceId;
    static uint32_t ms_bkgndColor;
    static int ms_enableCursor;
    static uint32_t ms_westonConfigNextSection;
}; //ControllerTests

int ControllerTests::ms_idAgentModuleValue = 0;
int ControllerTests::ms_inputControllerModuleValue = 0;
uint32_t ControllerTests::ms_screenIdOffset = 0;
uint32_t ControllerTests::ms_screenId = 0;
char *ControllerTests::ms_iviClientName = nullptr;
char *ControllerTests::ms_debugScopes = nullptr;
char *ControllerTests::ms_screenName = nullptr;
int32_t ControllerTests::ms_bkgndSurfaceId = 0;
uint32_t ControllerTests::ms_bkgndColor = 0;
int ControllerTests::ms_enableCursor = 0;
uint32_t ControllerTests::ms_westonConfigNextSection = 0;

static int custom_id_agent_module_init(struct weston_compositor *compositor, const struct ivi_layout_interface *interface)
{
    return ControllerTests::ms_idAgentModuleValue;
}
static int custom_input_controller_module_init(struct ivishell *shell)
{
    return ControllerTests::ms_inputControllerModuleValue;
}

static int custom_weston_config_section_get_uint(struct weston_config_section *section, const char *key, uint32_t *value, uint32_t default_value)
{
    if(strcmp(key, "screen-id-offset") == 0)
    {
        *value = ControllerTests::ms_screenIdOffset;
    }
    else if(strcmp(key, "screen-id") == 0)
    {
        *value = ControllerTests::ms_screenId;
    }
    return 0;
}

static int custom_weston_config_section_get_string(struct weston_config_section *section, const char *key, char **value, const char *default_value)
{
    if(strcmp(key, "ivi-client-name") == 0)
    {
        *value = (ControllerTests::ms_iviClientName == nullptr) ? nullptr : strdup(ControllerTests::ms_iviClientName);
    }
    else if(strcmp(key, "debug-scopes") == 0)
    {
        *value = (ControllerTests::ms_debugScopes == nullptr) ? nullptr : strdup(ControllerTests::ms_debugScopes);
    }
    else if(strcmp(key, "screen-name") == 0)
    {
        *value = (ControllerTests::ms_screenName == nullptr) ? nullptr : strdup(ControllerTests::ms_screenName);
    }
    return 0;
}

static int custom_weston_config_section_get_int(struct weston_config_section *section, const char *key, int32_t *value, int32_t default_value)
{
    if(strcmp(key, "bkgnd-surface-id") == 0)
    {
        *value = ControllerTests::ms_bkgndSurfaceId;
    }
    return 0;
}

static int custom_weston_config_section_get_color(struct weston_config_section *section, const char *key, uint32_t *color, uint32_t default_color)
{
    if(strcmp(key, "bkgnd-color") == 0)
    {
        *color = ControllerTests::ms_bkgndColor;
    }
    return 0;
}

static int custom_weston_config_section_get_bool(struct weston_config_section *section, const char *key, int *value, int default_value)
{
    if(strcmp(key, "enable-cursor") == 0)
    {
        *value = ControllerTests::ms_enableCursor;
    }
    return 0;
}

static int custom_weston_config_next_section(struct weston_config *config, struct weston_config_section **section, const char **name)
{
    switch (ControllerTests::ms_westonConfigNextSection)
    {
    case 0:
    {
        *name = "ivi-screen";
        ControllerTests::ms_westonConfigNextSection ++;
        return 1;
    }
    case 1:
    {
        *name = nullptr;
    }
    default:
        break;
    }
    return 0;
}

TEST_F(ControllerTests, send_surface_prop_noEvents)
{
    // Prepare the event mask of ivi surface 0 is 0
    m_layoutSurfaceProperties[0].event_mask = 0;
    // Invoke the send_surface_prop callback
    send_surface_prop(&mp_iviSurface[0]->property_changed, nullptr);
    /* When the send_surface_prop invokes, it will send the change of properties to client registed the event
     * With event mask is 0, there is no message send to client
     */
    ASSERT_EQ(get_id_of_surface_fake.call_count, 1);
    ASSERT_EQ(wl_resource_get_user_data_fake.call_count, 1);
    ASSERT_EQ(wl_resource_post_event_fake.call_count, 0);
}

TEST_F(ControllerTests, send_surface_prop_hasEvents)
{
    // Prepare the event mask of ivi surface 0 is the changed of all properties
    m_layoutSurfaceProperties[0].event_mask = IVI_NOTIFICATION_OPACITY | IVI_NOTIFICATION_SOURCE_RECT |
                                   IVI_NOTIFICATION_DEST_RECT | IVI_NOTIFICATION_VISIBILITY|
                                   IVI_NOTIFICATION_CONFIGURE;
    // Prepare fake for surface_get_weston_surface and wl_resource_get_user_data, make width and height of surface avaiable
    struct weston_surface l_westonSurface;
    l_westonSurface.width = 1920;
    l_westonSurface.height = 1080;
    struct weston_surface *lp_westonSurface = &l_westonSurface;
    SET_RETURN_SEQ(surface_get_weston_surface, &lp_westonSurface, 1);
    SET_RETURN_SEQ(wl_resource_get_user_data, (void**)&mp_iviController[0], 1);
    // Invoke the send_surface_prop
    send_surface_prop(&mp_iviSurface[0]->property_changed, nullptr);
    /* Check the logic
     * With event mask is mask all flags, the sever send 5 messages to client
     */
    ASSERT_EQ(get_id_of_surface_fake.call_count, 1);
    ASSERT_EQ(wl_resource_get_user_data_fake.call_count, 1);
    ASSERT_EQ(wl_resource_post_event_fake.call_count, 5);
}

TEST_F(ControllerTests, send_layer_prop_noEvents)
{
    // Prepare the event mask of ivi layer 0 is 0
    m_layoutLayerProperties[0].event_mask = 0;
    // Invoke the send_layer_prop
    send_layer_prop(&mp_iviLayer[0]->property_changed, nullptr);
    /* When the send_layer_prop invokes, it will send the change of properties to client registed the event
     * With event mask is 0, there is no message send to client
     */
    ASSERT_EQ(get_id_of_layer_fake.call_count, 1);
    ASSERT_EQ(wl_resource_get_user_data_fake.call_count, 1);
    ASSERT_EQ(wl_resource_post_event_fake.call_count, 0);
}

TEST_F(ControllerTests, send_layer_prop_hasEvents)
{
    // Prepare the event mask of ivi layer 0 is the changed of all properties
    m_layoutLayerProperties[0].event_mask = IVI_NOTIFICATION_OPACITY | IVI_NOTIFICATION_SOURCE_RECT |
                                            IVI_NOTIFICATION_DEST_RECT | IVI_NOTIFICATION_VISIBILITY;
    // Prepare fake for wl_resource_get_user_data, to return the ivi controller 0
    SET_RETURN_SEQ(wl_resource_get_user_data, (void**)&mp_iviController[0], 1);
    // Invoke the send_layer_prop
    send_layer_prop(&mp_iviLayer[0]->property_changed, nullptr);
    /* Check the logic
     * With event mask is mask all flags, the sever send 4 messages to client
     */
    ASSERT_EQ(get_id_of_layer_fake.call_count, 1);
    ASSERT_EQ(wl_resource_get_user_data_fake.call_count, 1);
    ASSERT_EQ(wl_resource_post_event_fake.call_count, 4);
}

TEST_F(ControllerTests, surface_event_create_idDifferentBkgnSurfaceId)
{
    // Prepare layout surface with id is 2, different with bkgnd_surface_id
    // Prepare fake for surface_get_weston_surface, to access commit_signal member
    struct ivi_layout_surface l_layoutSurface;
    l_layoutSurface.id_surface = 2;
    mp_iviShell->bkgnd_surface_id = 1;
    struct weston_surface l_westonSurface;
    struct weston_surface *lp_westonSurface = &l_westonSurface;
    SET_RETURN_SEQ(surface_get_weston_surface, &lp_westonSurface, 1);
    //Invoke the surface_event_create
    surface_event_create(&mp_iviShell->surface_created, &l_layoutSurface);
    /* Check the logic
     * With new id surface is different background surface id
     * + The ivisurface_created_signal need to trigger
     * + New surface will add to list surface of shell
     * + The server need to nofity to all controller client about the surface create callback
     * + Register the surface properties change event from ivi-layout
     * + The bkgnd_surface should not change
     */
    EXPECT_EQ(get_id_of_surface_fake.call_count, 1);
    EXPECT_EQ(MAX_NUMBER, g_SurfaceCreatedCount);
    EXPECT_EQ(get_properties_of_surface_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 1);
    EXPECT_EQ(surface_get_weston_surface_fake.call_count, 1);
    EXPECT_EQ(wl_list_insert_fake.call_count, 2);
    EXPECT_EQ(surface_add_listener_fake.call_count, 1);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, MAX_NUMBER);
    struct ivisurface *lp_iviSurf = (struct ivisurface*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivisurface, notification_list));
    EXPECT_EQ(lp_iviSurf->shell, mp_iviShell);
    EXPECT_EQ(lp_iviSurf->layout_surface, &l_layoutSurface);
    EXPECT_EQ(mp_iviShell->bkgnd_surface, nullptr);
    // free resource
    free(lp_iviSurf);
}

TEST_F(ControllerTests, surface_event_create_idSameBkgnSurfaceId)
{
    // Prepare layout surface with id is 1, different with bkgnd_surface_id
    // Prepare fake for surface_get_weston_surface, to access commit_signal member
    struct ivi_layout_surface l_layoutSurface;
    l_layoutSurface.id_surface = 1;
    mp_iviShell->bkgnd_surface_id = 1;
    struct weston_surface l_westonSurface;
    struct weston_surface *lp_westonSurface = &l_westonSurface;
    SET_RETURN_SEQ(surface_get_weston_surface, &lp_westonSurface, 1);
    //Invoke the surface_event_create
    surface_event_create(&mp_iviShell->surface_created, &l_layoutSurface);
    /* Check the logic
     * With new id surface is same with background surface id
     * + The ivisurface_created_signal should not trigger
     * + New surface will not add to list surface of shell
     * + The server should not nofity to all controller client about the surface create callback
     * + The bkgnd_surface need to change
     */
    EXPECT_EQ(get_id_of_surface_fake.call_count, 1);
    EXPECT_EQ(0, g_SurfaceCreatedCount);
    EXPECT_EQ(get_properties_of_surface_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 1);
    EXPECT_EQ(surface_get_weston_surface_fake.call_count, 1);
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(surface_add_listener_fake.call_count, 0);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, 0);
    struct ivisurface *lp_iviSurf = (struct ivisurface*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivisurface, notification_list));
    EXPECT_EQ(lp_iviSurf->shell, mp_iviShell);
    EXPECT_EQ(lp_iviSurf->layout_surface, &l_layoutSurface);
    EXPECT_EQ(mp_iviShell->bkgnd_surface, lp_iviSurf);
    //free resource
    free(lp_iviSurf);
}

TEST_F(ControllerTests, layer_event_create)
{
    // Prepare data input, layout layer with id layer is 10
    struct ivi_layout_layer l_layoutLayer;
    l_layoutLayer.id_layer = 10;
    // Invoke the layer_event_create
    layer_event_create(&mp_iviShell->layer_created, &l_layoutLayer);
    /* Check the logic
     * + New layer will add to list layer of shell
     * + The server need to nofity to all controller client about the layer create callback
     * + Register the layer properties change event from ivi-layout
     */
    EXPECT_EQ(get_id_of_layer_fake.call_count, 1);
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 1);
    EXPECT_EQ(get_properties_of_layer_fake.call_count, 1);
    EXPECT_EQ(layer_add_listener_fake.call_count, 1);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, MAX_NUMBER);
    struct ivilayer *lp_iviLayer = (struct ivilayer*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivilayer, notification_list));
    EXPECT_EQ(lp_iviLayer->shell, mp_iviShell);
    EXPECT_EQ(lp_iviLayer->layout_layer, &l_layoutLayer);
    //free resource
    free(lp_iviLayer);
}

TEST_F(ControllerTests, output_created_event_customScreen)
{
    // Prepare data input, weston output with name is "custom_screen" and id is 1
    struct weston_output l_createdOutput;
    l_createdOutput.id = 1;
    l_createdOutput.name = (char*)"custom_screen";
    // Invoke the output_created_event
    output_created_event(&mp_iviShell->output_created, &l_createdOutput);
    /* Check the logic of custom screen (don't have in the list of screen id)
     * + New screen will add to list screen of shell
     * + id of new screen should be equal sum of weston_output->id and screen_if_offset
     * + weston_compositor_schedule_repaint should trigger one time
     */
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 1);
    EXPECT_EQ(weston_compositor_schedule_repaint_fake.call_count, 1);
    struct iviscreen *lp_iviScreen = (struct iviscreen*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct iviscreen, resource_list));
    EXPECT_EQ(lp_iviScreen->shell, mp_iviShell);
    EXPECT_EQ(lp_iviScreen->output, &l_createdOutput);
    EXPECT_EQ(lp_iviScreen->id_screen, mp_iviShell->screen_id_offset + l_createdOutput.id);
    // free resource
    free(lp_iviScreen);
}

TEST_F(ControllerTests, output_created_event_defaultConfigScreen)
{
    // Prepare data input, weston output with name same with screen info default
    struct weston_output l_createdOutput;
    l_createdOutput.id = 1000;
    l_createdOutput.name = mp_screenInfo->screen_name;
    // Invoke the output_created_event
    output_created_event(&mp_iviShell->output_created, &l_createdOutput);
    /* Check the logic of default screen (screen info in the list of screen_id)
     * + New screen will add to list screen of shell
     * + id of new screen should be screen_info->id
     * + weston_compositor_schedule_repaint should trigger one time
     */
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 1);
    EXPECT_EQ(weston_compositor_schedule_repaint_fake.call_count, 1);
    struct iviscreen *lp_iviScreen = (struct iviscreen*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct iviscreen, resource_list));
    EXPECT_EQ(lp_iviScreen->shell, mp_iviShell);
    EXPECT_EQ(lp_iviScreen->output, &l_createdOutput);
    EXPECT_EQ(lp_iviScreen->id_screen, mp_screenInfo->screen_id);
    // free resource
    free(lp_iviScreen);
}

TEST_F(ControllerTests, bind_ivi_controller)
{
    // Prepare fake for wl_resource_create, make it different null
    SET_RETURN_SEQ(wl_resource_create, &mp_wlResourceDefault, 1);
    // Invoke the bind_ivi_controller
    bind_ivi_controller(mp_fakeClient, mp_iviShell, 1, 1);
    /* Check the logic
     * + wl_resource_create and wl_resource_set_implementation should trigger one time
     * + new controller should be added to list controller of shell
     * + The controller client need to get the list avaiable of surfaces and layers
     */
    EXPECT_EQ(wl_resource_create_fake.call_count, 1);
    EXPECT_EQ(wl_resource_set_implementation_fake.call_count, 1);
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(wl_list_init_fake.call_count, 2);
    EXPECT_EQ(get_id_of_surface_fake.call_count, MAX_NUMBER);
    EXPECT_EQ(get_id_of_layer_fake.call_count, MAX_NUMBER);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, MAX_NUMBER*2);
    struct ivicontroller *lp_iviController = (struct ivicontroller*)(uintptr_t(wl_list_init_fake.arg0_history[0]) - offsetof(struct ivicontroller, surface_notifications));
    EXPECT_EQ(lp_iviController->shell, mp_iviShell);
    EXPECT_EQ(lp_iviController->client, mp_fakeClient);
    EXPECT_EQ(lp_iviController->id, 1);
    // free resource
    free(lp_iviController);
}

TEST_F(ControllerTests, controller_create_screen_wrongWestonHead)
{
    // Prepare fake the wl_resource_get_user_data, to return wrong weston head and ivi wm controller 0
    struct weston_head l_westonHead;
    struct weston_output *lp_fakeWestonOutput = (struct weston_output*)0xFFFFFFFF;
    l_westonHead.output = lp_fakeWestonOutput;
    void *lpp_getUserData [] = {&l_westonHead, mp_iviController[0]};
    SET_RETURN_SEQ(wl_resource_get_user_data, lpp_getUserData, 2);
    // Invoke the controller_create_screen
    controller_create_screen(nullptr, nullptr, nullptr, 1);
    /* Check the logic when got the wrong weston_output
     * + wl_resource_create, wl_resource_set_implementation, wl_list_insert and wl_resource_post_event should not invoke
     */
    ASSERT_EQ(wl_resource_create_fake.call_count, 0);
    EXPECT_EQ(wl_resource_set_implementation_fake.call_count, 0);
    EXPECT_EQ(wl_list_insert_fake.call_count, 0);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, 0);
}

TEST_F(ControllerTests, controller_create_screen_correctWestonHead)
{
    // Prepare fake the wl_resource_get_user_data, to return weston head 0 and ivi wm controller 0
    struct weston_head l_westonHead;
    l_westonHead.output = &m_westonOutput[0];
    void *lpp_getUserData [] = {&l_westonHead, mp_iviController[0]};
    SET_RETURN_SEQ(wl_resource_get_user_data, lpp_getUserData, 2);
    // Prepare fake for wl_resource_create, make it different null
    SET_RETURN_SEQ(wl_resource_create, &mp_wlResourceDefault, 1);
    // Invoke the controller_create_screen
    controller_create_screen(nullptr, nullptr, nullptr, 1);
    /* Check the logic when got the right weston_output
     * + wl_resource_create, wl_resource_set_implementation, wl_list_insert need trigger 1 time
     * + The client need to got the event of screen id and screen name
     */
    ASSERT_EQ(wl_resource_create_fake.call_count, 1);
    EXPECT_EQ(wl_resource_set_implementation_fake.call_count, 1);
    EXPECT_EQ(wl_list_insert_fake.call_count, 1);
    EXPECT_EQ(wl_resource_post_event_fake.call_count, 2);
}

TEST_F(ControllerTests, wet_module_init_cannotGetIviLayoutInterface)
{
    // Don't fake the result of weston_plugin_api_get, it will return null
    // Invoke the wet_module_init, expected a failure
    ASSERT_NE(wet_module_init(&m_westonCompositor, nullptr, nullptr), 0);
    /* Check the logic in case the function can't get the ivi layout interface
     * weston_plugin_api_get should trigger one time
     * wet_get_config should not trigger
     */
    ASSERT_EQ(weston_plugin_api_get_fake.call_count, 1);
    ASSERT_EQ(wet_get_config_fake.call_count, 0);
}

TEST_F(ControllerTests, wet_module_init_cannotCreateGlobalWmIviInterface)
{
    // Prepare fake for weston_plugin_api_get, to return the ivi layout interface
    struct ivi_layout_interface *lp_iviLayoutInterface = &g_iviLayoutInterfaceFake;
    SET_RETURN_SEQ(weston_plugin_api_get, (const void**)&lp_iviLayoutInterface, 1);
    // Don't prepare fake for wl_global_create, it will return nullptr
    // Invoke the wet_module_init, expected a failure
    ASSERT_NE(wet_module_init(&m_westonCompositor, nullptr, nullptr), 0);
    /* Check the logic in case the function can't create wl_global wm-ivi controller
     * weston_plugin_api_get should trigger once time
     * wl_global_create should trigger once time
     * wet_load_module_entrypoint should not trigger
     */
    ASSERT_EQ(weston_plugin_api_get_fake.call_count, 1);
    ASSERT_EQ(wl_global_create_fake.call_count, 1);
    ASSERT_EQ(wet_load_module_entrypoint_fake.call_count, 0);
}

TEST_F(ControllerTests, wet_module_init_cannotInitIviInputModule)
{
    // Prepare fake for weston_plugin_api_get and wl_global_create, to return valid values
    struct ivi_layout_interface *lp_iviLayoutInterface = &g_iviLayoutInterfaceFake;
    SET_RETURN_SEQ(weston_plugin_api_get, (const void**)&lp_iviLayoutInterface, 1);
    struct wl_global *lp_wlGlobal = (struct wl_global*) 0xFFFFFFFF;
    SET_RETURN_SEQ(wl_global_create, &lp_wlGlobal, 1);
    // Prepare fake for and wet_load_module_entrypoint, to make the ivi-input-init function callback
    // expect the function callback will return a failure
    ControllerTests::ms_inputControllerModuleValue = -1;
    void *lp_iviInputInitModule = (void*)custom_input_controller_module_init;
    SET_RETURN_SEQ(wet_load_module_entrypoint, &lp_iviInputInitModule, 1);
    // Invoke the wet_module_init, expected a failure
    ASSERT_NE(wet_module_init(&m_westonCompositor, nullptr, nullptr), 0);
    /* Check the logic in case the function can't init the ivi-input-module
     * weston_plugin_api_get should trigger once time
     * wl_global_create should trigger once time
     * wet_load_module_entrypoint should trigger once time
     */
    ASSERT_EQ(weston_plugin_api_get_fake.call_count, 1);
    ASSERT_EQ(wl_global_create_fake.call_count, 1);
    ASSERT_EQ(wet_load_module_entrypoint_fake.call_count, 1);
}

TEST_F(ControllerTests, wet_module_init_cannotGetWestonConfig)
{
    enable_utility_funcs_of_array_list();
    // Prepare fake for weston_plugin_api_get and wl_global_create, to return valid values
    struct ivi_layout_interface *lp_iviLayoutInterface = &g_iviLayoutInterfaceFake;
    SET_RETURN_SEQ(weston_plugin_api_get, (const void**)&lp_iviLayoutInterface, 1);
    struct wl_global *lp_wlGlobal = (struct wl_global*) 0xFFFFFFFF;
    SET_RETURN_SEQ(wl_global_create, &lp_wlGlobal, 1);
    // Prepare fake for and wet_load_module_entrypoint, to make the ivi-input-init and id-agent-init function callbacks
    // Setup ivi-input-init return success, id-agent-init return a failure
    ControllerTests::ms_inputControllerModuleValue = 0;
    ControllerTests::ms_idAgentModuleValue = -1;
    void *lp_iviInputInitModule[] ={(void *)custom_input_controller_module_init, (void *)custom_id_agent_module_init};
    SET_RETURN_SEQ(wet_load_module_entrypoint, lp_iviInputInitModule, 2);
    // Invoke the wet_module_init, expected a success when can't init the id-agent module
    ASSERT_EQ(wet_module_init(&m_westonCompositor, nullptr, nullptr), 0);
    /* Check the logic in case the function can't get wet_get_config
     * weston_plugin_api_get should trigger once time
     * wl_global_create should trigger once time
     * wet_load_module_entrypoint should trigger two times
     */
    EXPECT_EQ(weston_plugin_api_get_fake.call_count, 1);
    EXPECT_EQ(wl_global_create_fake.call_count, 1);
    EXPECT_EQ(wet_load_module_entrypoint_fake.call_count, 2);
    /* In the case the wet_module_init success, need to check the member of ivi-shell
     * layout interface need to be set
     * wet_get_config can't get, so the properties of ivi shell need to setup
     */
    struct ivishell *lp_iviShell = (struct ivishell*)wl_global_create_fake.arg3_history[0];
    EXPECT_EQ(lp_iviShell->interface, lp_iviLayoutInterface);
    EXPECT_EQ(lp_iviShell->screen_id_offset, 0);
    EXPECT_EQ(lp_iviShell->ivi_client_name, nullptr);
    EXPECT_EQ(lp_iviShell->bkgnd_surface_id, 0);
    EXPECT_EQ(lp_iviShell->debug_scopes, nullptr);
    EXPECT_EQ(lp_iviShell->bkgnd_color, 0);
    EXPECT_EQ(lp_iviShell->enable_cursor, 0);
    EXPECT_EQ(lp_iviShell->screen_ids.size, 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_surface), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_layer), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_screen), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_controller), 0);
    EXPECT_NE(lp_iviShell->layer_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->layer_removed.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_removed.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_configured.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_destroyed.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_resized.notify, nullptr);
    EXPECT_NE(lp_iviShell->destroy_listener.notify, nullptr);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_created_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_destroyed_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_resized_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.destroy_signal.listener_list), 1);
    // free resource
    free(lp_iviShell);
}

TEST_F(ControllerTests, wet_module_init_canGetWestonConfig)
{
    enable_utility_funcs_of_array_list();
    // Prepare fake for relevant functions, to make the wet_module_init return success
    struct ivi_layout_interface *lp_iviLayoutInterface = &g_iviLayoutInterfaceFake;
    struct wl_global *lp_wlGlobal = (struct wl_global*) 0xFFFFFFFF;
    struct weston_config *lp_westonConfig = (struct weston_config*) 0xFFFFFFFF;
    struct weston_config_section *lp_westonConfigSection = (struct weston_config_section*) 0xFFFFFFFF;
    int lpp_westonConfigNextSection[] = {1, 0};
    ControllerTests::ms_inputControllerModuleValue = 0;
    ControllerTests::ms_idAgentModuleValue = -1;
    void *lp_iviInputInitModule[] ={(void *)custom_input_controller_module_init, (void *)custom_id_agent_module_init};
    SET_RETURN_SEQ(weston_plugin_api_get, (const void**)&lp_iviLayoutInterface, 1);
    SET_RETURN_SEQ(wl_global_create, &lp_wlGlobal, 1);
    SET_RETURN_SEQ(wet_get_config, &lp_westonConfig, 1);
    SET_RETURN_SEQ(weston_config_get_section, &lp_westonConfigSection, 1);
    SET_RETURN_SEQ(weston_config_next_section, lpp_westonConfigNextSection, 2);
    SET_RETURN_SEQ(wet_load_module_entrypoint, lp_iviInputInitModule, 2);
    // Prepare the config via the custom fake function
    ControllerTests::ms_screenIdOffset = 100;
    ControllerTests::ms_screenId = 10;
    ControllerTests::ms_iviClientName = (char *)"ivi_client_app.elf";
    ControllerTests::ms_debugScopes = (char *)"controller";
    ControllerTests::ms_screenName = (char *)"screen_test";
    ControllerTests::ms_bkgndSurfaceId = 100;
    ControllerTests::ms_bkgndColor = 0xAABBCCDD;
    ControllerTests::ms_enableCursor = 1;
    ControllerTests::ms_westonConfigNextSection = 0;
    weston_config_section_get_uint_fake.custom_fake = custom_weston_config_section_get_uint;
    weston_config_section_get_string_fake.custom_fake = custom_weston_config_section_get_string;
    weston_config_section_get_int_fake.custom_fake = custom_weston_config_section_get_int;
    weston_config_section_get_color_fake.custom_fake = custom_weston_config_section_get_color;
    weston_config_section_get_bool_fake.custom_fake = custom_weston_config_section_get_bool;
    weston_config_next_section_fake.custom_fake = custom_weston_config_next_section;

    // Invoke the wet_module_init, expected a success when can't init the id-agent module
    ASSERT_EQ(wet_module_init(&m_westonCompositor, nullptr, nullptr), 0);
    /* Check the logic in case the function can read the weston config
     * weston_plugin_api_get should trigger once time
     * wl_global_create should trigger once time
     * wet_load_module_entrypoint should trigger two times
     * The weston_config_get_section and wet_get_config should trigger three times
     */
    EXPECT_EQ(weston_plugin_api_get_fake.call_count, 1);
    EXPECT_EQ(wl_global_create_fake.call_count, 1);
    EXPECT_EQ(wet_load_module_entrypoint_fake.call_count, 2);
    EXPECT_EQ(wet_get_config_fake.call_count, 3);
    EXPECT_EQ(weston_config_get_section_fake.call_count, 3);
    /* In the case the wet_module_init success, need to check the member of ivi-shell
     * layout interface need to be set
     * There is a default config, so the properties of ivi shell need to setup
     */
    struct ivishell *lp_iviShell = (struct ivishell*)wl_global_create_fake.arg3_history[0];
    EXPECT_EQ(lp_iviShell->interface, lp_iviLayoutInterface);
    EXPECT_EQ(lp_iviShell->screen_id_offset, ControllerTests::ms_screenIdOffset);
    EXPECT_EQ(strcmp(lp_iviShell->ivi_client_name, ControllerTests::ms_iviClientName), 0);
    EXPECT_EQ(lp_iviShell->bkgnd_surface_id, ControllerTests::ms_bkgndSurfaceId);
    EXPECT_EQ(strcmp(lp_iviShell->debug_scopes, ControllerTests::ms_debugScopes), 0);
    EXPECT_EQ(lp_iviShell->bkgnd_color, ControllerTests::ms_bkgndColor);
    EXPECT_EQ(lp_iviShell->enable_cursor, ControllerTests::ms_enableCursor);
    EXPECT_EQ(lp_iviShell->screen_ids.size, sizeof(struct screen_id_info));
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_surface), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_layer), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_screen), 0);
    EXPECT_EQ(wl_list_length(&lp_iviShell->list_controller), 0);
    EXPECT_NE(lp_iviShell->layer_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->layer_removed.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_removed.notify, nullptr);
    EXPECT_NE(lp_iviShell->surface_configured.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_created.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_destroyed.notify, nullptr);
    EXPECT_NE(lp_iviShell->output_resized.notify, nullptr);
    EXPECT_NE(lp_iviShell->destroy_listener.notify, nullptr);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_created_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_destroyed_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.output_resized_signal.listener_list), 1);
    EXPECT_EQ(wl_list_length(&m_westonCompositor.destroy_signal.listener_list), 1);
    // free resource
    struct screen_id_info *screen_info = NULL;
    wl_array_for_each(screen_info, &lp_iviShell->screen_ids) {
        free(screen_info->screen_name);
    }
    wl_array_release(&lp_iviShell->screen_ids);
    free(lp_iviShell->ivi_client_name);
    free(lp_iviShell->debug_scopes);
    free(lp_iviShell);
}
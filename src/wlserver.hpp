// Wayland stuff

#pragma once

#include <wayland-server-core.h>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <set>
#include <list>
#include <optional>
#include <vulkan/vulkan_core.h>

#include "drm.hpp"
#include "steamcompmgr_shared.hpp"

#define WLSERVER_BUTTON_COUNT 7

struct _XDisplay;
struct xwayland_ctx_t;

struct wlserver_vk_swapchain_feedback
{
	uint32_t image_count;
	VkFormat vk_format;
	VkColorSpaceKHR vk_colorspace;
	VkCompositeAlphaFlagBitsKHR vk_composite_alpha;
	VkSurfaceTransformFlagBitsKHR vk_pre_transform;
	VkPresentModeKHR vk_present_mode;
	VkBool32 vk_clipped;

	std::shared_ptr<wlserver_hdr_metadata> hdr_metadata_blob;
};

struct ResListEntry_t {
	struct wlr_surface *surf;
	struct wlr_buffer *buf;
	bool async;
	std::shared_ptr<wlserver_vk_swapchain_feedback> feedback;
	std::vector<struct wl_resource*> presentation_feedbacks;
};

struct wlserver_content_override;

class gamescope_xwayland_server_t
{
public:
	gamescope_xwayland_server_t(wl_display *display);
	~gamescope_xwayland_server_t();

	void on_xwayland_ready(void *data);
	static void xwayland_ready_callback(struct wl_listener *listener, void *data);

	bool is_xwayland_ready() const;
	const char *get_nested_display_name() const;

	void set_wl_id( struct wlserver_x11_surface_info *surf, uint32_t id );

	_XDisplay *get_xdisplay();

	std::unique_ptr<xwayland_ctx_t> ctx;

	void wayland_commit(struct wlr_surface *surf, struct wlr_buffer *buf);

	std::vector<ResListEntry_t> retrieve_commits();

	void handle_override_window_content( struct wl_client *client, struct wl_resource *resource, struct wl_resource *surface_resource, uint32_t x11_window );
	void destroy_content_override( struct wlserver_x11_surface_info *x11_surface, struct wlr_surface *surf);
	void destroy_content_override(struct wlserver_content_override *co);

	struct wl_client *get_client();
	struct wlr_output *get_output();

	void update_output_info();

private:
	struct wlr_xwayland_server *xwayland_server = NULL;
	struct wl_listener xwayland_ready_listener = { .notify = xwayland_ready_callback };

	struct wlr_output *output;

	std::map<uint32_t, wlserver_content_override *> content_overrides;

	bool xwayland_ready = false;
	_XDisplay *dpy = NULL;

	std::mutex wayland_commit_lock;
	std::vector<ResListEntry_t> wayland_commit_queue;
};

struct wlserver_t {
	struct wl_display *display;
	struct wl_event_loop *event_loop;
	char wl_display_name[32];

	struct {
		struct wlr_backend *multi_backend;
		struct wlr_backend *headless_backend;
		struct wlr_backend *libinput_backend;

		struct wlr_renderer *renderer;
		struct wlr_compositor *compositor;
		struct wlr_session *session;
		struct wlr_seat *seat;

		// Used to simulate key events and set the keymap
		struct wlr_keyboard *virtual_keyboard_device;

		std::vector<std::unique_ptr<gamescope_xwayland_server_t>> xwayland_servers;
	} wlr;
	
	struct wlr_surface *mouse_focus_surface;
	struct wlr_surface *kb_focus_surface;
	double mouse_surface_cursorx = 0.0f;
	double mouse_surface_cursory = 0.0f;
	
	bool button_held[ WLSERVER_BUTTON_COUNT ];
	std::set <uint32_t> touch_down_ids;

	struct {
		char *name;
		char *description;
		int phys_width, phys_height; // millimeters
	} output_info;

	struct wl_listener session_active;
	struct wl_listener new_input_method;

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_surface;
	std::vector<std::shared_ptr<steamcompmgr_win_t>> xdg_wins;
	std::atomic<bool> xdg_dirty;
	std::mutex xdg_commit_lock;
	std::vector<ResListEntry_t> xdg_commit_queue;
};

std::vector<ResListEntry_t> wlserver_xdg_commit_queue();

struct wlserver_keyboard {
	struct wlr_keyboard *wlr;
	
	struct wl_listener modifiers;
	struct wl_listener key;
};

struct wlserver_pointer {
	struct wlr_pointer *wlr;
	
	struct wl_listener motion;
	struct wl_listener button;
	struct wl_listener axis;
	struct wl_listener frame;
};

struct wlserver_touch {
	struct wlr_touch *wlr;
	
	struct wl_listener down;
	struct wl_listener up;
	struct wl_listener motion;
};

enum wlserver_touch_click_mode {
	WLSERVER_TOUCH_CLICK_HOVER = 0,
	WLSERVER_TOUCH_CLICK_LEFT = 1,
	WLSERVER_TOUCH_CLICK_RIGHT = 2,
	WLSERVER_TOUCH_CLICK_MIDDLE = 3,
	WLSERVER_TOUCH_CLICK_PASSTHROUGH = 4,
	WLSERVER_TOUCH_CLICK_DISABLED = 5,
	WLSERVER_TOUCH_CLICK_TRACKPAD = 6,
};

extern enum wlserver_touch_click_mode g_nDefaultTouchClickMode;
extern enum wlserver_touch_click_mode g_nTouchClickMode;

void xwayland_surface_commit(struct wlr_surface *wlr_surface);

bool wlsession_init( void );
int wlsession_open_kms( const char *device_name );

bool wlserver_init( void );

void wlserver_run(void);

void wlserver_lock(void);
void wlserver_unlock(void);
bool wlserver_is_lock_held(void);

void wlserver_keyboardfocus( struct wlr_surface *surface );
void wlserver_key( uint32_t key, bool press, uint32_t time );

void wlserver_mousefocus( struct wlr_surface *wlrsurface, int x = 0, int y = 0 );
void wlserver_mousemotion( int x, int y, uint32_t time );
void wlserver_mousewarp( int x, int y, uint32_t time );
void wlserver_mousebutton( int button, bool press, uint32_t time );
void wlserver_mousewheel( int x, int y, uint32_t time );

void wlserver_touchmotion( double x, double y, int touch_id, uint32_t time );
void wlserver_touchdown( double x, double y, int touch_id, uint32_t time );
void wlserver_touchup( int touch_id, uint32_t time );

void wlserver_send_frame_done( struct wlr_surface *surf, const struct timespec *when );

bool wlserver_surface_is_async( struct wlr_surface *surf );
const std::shared_ptr<wlserver_vk_swapchain_feedback>& wlserver_surface_swapchain_feedback( struct wlr_surface *surf );

std::vector<std::shared_ptr<steamcompmgr_win_t>> wlserver_get_xdg_shell_windows();
bool wlserver_xdg_dirty();

struct wlserver_output_info {
	const char *description;
	int phys_width, phys_height; // millimeters
};

void wlserver_set_output_info( const wlserver_output_info *info );

gamescope_xwayland_server_t *wlserver_get_xwayland_server( size_t index );
const char *wlserver_get_wl_display_name( void );

struct wlserver_wl_surface_info
{
	wlserver_x11_surface_info *x11_surface = nullptr;
	wlserver_xdg_surface_info *xdg_surface = nullptr;


	struct wlr_surface *wlr = nullptr;
	struct wl_listener commit;
	struct wl_listener destroy;

	uint32_t presentation_hint = 0;

	std::shared_ptr<wlserver_vk_swapchain_feedback> swapchain_feedback = {};

	uint64_t sequence = 0;
	std::vector<struct wl_resource*> pending_presentation_feedbacks;
};

void wlserver_x11_surface_info_init( struct wlserver_x11_surface_info *surf, gamescope_xwayland_server_t *server, uint32_t x11_id );
void wlserver_x11_surface_info_finish( struct wlserver_x11_surface_info *surf );

void wlserver_set_xwayland_server_mode( size_t idx, int w, int h, int refresh );

extern std::atomic<bool> g_bPendingTouchMovement;

void wlserver_open_steam_menu( bool qam );

uint32_t wlserver_make_new_xwayland_server();
void wlserver_destroy_xwayland_server(gamescope_xwayland_server_t *server);

void wlserver_presentation_feedback_presented( struct wlr_surface *surface, std::vector<struct wl_resource*>& presentation_feedbacks, uint64_t last_refresh_nsec, int refresh_rate );
void wlserver_presentation_feedback_discard( struct wlr_surface *surface, std::vector<struct wl_resource*>& presentation_feedbacks );

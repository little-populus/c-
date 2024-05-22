#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// Function to create a simple window with specified visual and depth
Window create_simple_window(Display* display, int width, int height, int depth, Visual* visual)
{
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    XSetWindowAttributes attrs;
    attrs.colormap = XCreateColormap(display, root, visual, AllocNone);
    attrs.border_pixel = 0;
    attrs.background_pixel = 0;
    Window window = XCreateWindow(display, root, 0, 0, width, height, 0, depth, InputOutput, visual,
        CWColormap | CWBorderPixel | CWBackPixel, &attrs);
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    return window;
    pthread_exit(nullptr);
}

// Function to print window attributes
void print_window_attributes(Display* display, Window window, const char* window_name)
{
    XWindowAttributes attr;
    if (XGetWindowAttributes(display, window, &attr)) {
        printf("Attributes of %s:\n", window_name);
        printf("  Width: %d\n", attr.width);
        printf("  Height: %d\n", attr.height);
        printf("  X: %d\n", attr.x);
        printf("  Y: %d\n", attr.y);
        printf("  Border width: %d\n", attr.border_width);
        printf("  Depth: %d\n", attr.depth);
        printf("  Visual: %p\n", (void*)attr.visual);
        printf("  Root: %lu\n", attr.root);
        printf("  Bit gravity: %d\n", attr.bit_gravity);
        printf("  Win gravity: %d\n", attr.win_gravity);
        printf("  Backing store: %d\n", attr.backing_store);
        printf("  Backing planes: %lu\n", attr.backing_planes);
        printf("  Backing pixel: %lu\n", attr.backing_pixel);
        printf("  Save under: %d\n", attr.save_under);
        printf("  Colormap: %lu\n", attr.colormap);
        printf("  Map installed: %d\n", attr.map_installed);
        printf("  Map state: %d\n", attr.map_state);
        printf("  All event masks: %lu\n", attr.all_event_masks);
        printf("  Your event mask: %lu\n", attr.your_event_mask);
        printf("  Do not propagate mask: %lu\n", attr.do_not_propagate_mask);
        printf("  Override redirect: %d\n", attr.override_redirect);
        printf("  Screen: %p\n", (void*)attr.screen);
    } else {
        fprintf(stderr, "Unable to get attributes of %s\n", window_name);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <window-id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        exit(EXIT_FAILURE);
    }

    Window target_window = (Window)strtol(argv[1], NULL, 0);
    if (!target_window) {
        fprintf(stderr, "Invalid window ID\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // Ensure the Xcomposite extension is available
    int event_base, error_base;
    if (!XCompositeQueryExtension(display, &event_base, &error_base)) {
        fprintf(stderr, "Xcomposite extension not available\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    // Redirect the target window's contents to an off-screen buffer
    XCompositeRedirectWindow(display, target_window, CompositeRedirectAutomatic);

    // Get the target window's geometry
    XWindowAttributes target_attr;
    XGetWindowAttributes(display, target_window, &target_attr);
    int width = target_attr.width;
    int height = target_attr.height;
    int depth = target_attr.depth;
    Visual* visual = target_attr.visual;

    // Create a simple window to display the captured content with the same visual and depth as the target window
    Window output_window = create_simple_window(display, width, height, depth, visual);

    // Print attributes of both windows
    print_window_attributes(display, target_window, "Target Window");
    print_window_attributes(display, output_window, "Output Window");

    // Create an XRender picture for the target window
    XRenderPictFormat* pict_format = XRenderFindVisualFormat(display, visual);
    if (!pict_format) {
        fprintf(stderr, "Unable to find XRender pict format\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    XRenderPictureAttributes pict_attr;
    memset(&pict_attr, 0, sizeof(pict_attr));
    Picture target_picture = XRenderCreatePicture(display, target_window, pict_format, 0, &pict_attr);

    // Create an XRender picture for the output window
    Picture output_picture = XRenderCreatePicture(display, output_window, pict_format, 0, &pict_attr);

    // Create a pixmap and picture for the off-screen buffer
    Pixmap pixmap = XCompositeNameWindowPixmap(display, target_window);
    Picture pixmap_picture = XRenderCreatePicture(display, pixmap, pict_format, 0, &pict_attr);

    // Main event loop to keep the output window updated
    while (1) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == Expose) {
            // Composite the target window's content to the output window
            XRenderComposite(display, PictOpSrc, pixmap_picture, None, output_picture, 0, 0, 0, 0, 0, 0, width,
                height);
        }

        if (event.type == KeyPress) {
            break; // Exit the loop on key press
        }
    }

    // Cleanup
    XRenderFreePicture(display, pixmap_picture);
    XRenderFreePicture(display, target_picture);
    XRenderFreePicture(display, output_picture);
    XFreePixmap(display, pixmap);
    XDestroyWindow(display, output_window);
    XCloseDisplay(display);

    return 0;
}

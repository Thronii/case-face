#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <@smallstoneapps/linked-list/linked-list.h>
#include "fctx-layer.h"
#include "logging.h"

struct FctxLayer {
    Layer *layer;
    FctxLayerUpdateProc update_proc;
    FctxLayer *parent;
    LinkedRoot *children;
    void *data;
};

static bool prv_layer_children_foreach(void *obj, void *fctx) {
    logf();
    FctxLayer *this = (FctxLayer *) obj;
    bool hidden = fctx_layer_get_hidden(this);
    if (this->update_proc && !hidden) {
        fctx_set_scale(fctx, FPointOne, FPointOne);
        fctx_set_rotation(fctx, 0);

        GRect frame = fctx_layer_get_frame(this);
        if (this->parent != NULL) {
            GRect parent = fctx_layer_get_frame(this->parent);
            frame.origin.x += parent.origin.x;
            frame.origin.y += parent.origin.y;
        }
        fctx_set_offset(fctx, g2fpoint(frame.origin));

        this->update_proc(this, fctx);
    }
    if (this->children && !hidden) linked_list_foreach(this->children, prv_layer_children_foreach, fctx);
    return true;
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
    logf();
    FctxLayer *this = layer_get_data(layer);

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    linked_list_foreach(this->children, prv_layer_children_foreach, &fctx);
    fctx_deinit_context(&fctx);
}

FctxLayer *window_get_root_fctx_layer(const Window *window) {
    logf();
    Layer *root_layer = window_get_root_layer(window);
    FctxLayer *this = fctx_layer_create(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT));
    layer_set_update_proc(this->layer, prv_update_proc);
    layer_add_child(root_layer, this->layer);
    this->children = linked_list_create_root();
    return this;
}

FctxLayer *fctx_layer_create(const GRect frame) {
    logf();
    Layer *layer = layer_create_with_data(frame, sizeof(FctxLayer));
    FctxLayer *this = layer_get_data(layer);
    this->layer = layer;
    this->update_proc = NULL;
    this->parent = NULL;
    this->children = NULL;
    this->data = NULL;
    return this;
}

FctxLayer *fctx_layer_create_with_data(const GRect frame, const size_t size) {
    logf();
    FctxLayer *this = fctx_layer_create(frame);
    this->data = malloc(size);
    return this;
}

void fctx_layer_set_update_proc(FctxLayer *this, FctxLayerUpdateProc update_proc) {
    logf();
    this->update_proc = update_proc;
}

void fctx_layer_destroy(FctxLayer *this) {
    logf();
    fctx_layer_remove_from_parent(this);

    if (this->data) free(this->data);
    this->data = NULL;

    if (this->children) free(this->children);
    this->children = NULL;

    this->update_proc = NULL;

    Layer *layer = this->layer;
    this->layer = NULL;
    layer_destroy(layer);
}

Layer *fctx_layer_get_layer(const FctxLayer *this) {
    logf();
    return this->layer;
}

void *fctx_layer_get_data(const FctxLayer *this) {
    logf();
    return this->data;
}

void fctx_layer_add_child(FctxLayer *this, FctxLayer *child) {
    logf();
    if (!this->children) this->children = linked_list_create_root();
    linked_list_append(this->children, child);
    child->parent = this;
    layer_add_child(this->layer, child->layer);
}

void fctx_layer_remove_from_parent(FctxLayer *this) {
    logf();
    if (!this->parent) return;

    FctxLayer *parent = this->parent;
    int16_t index = linked_list_find(parent->children, this);
    if (index > -1) {
        layer_remove_from_parent(this->layer);
        linked_list_remove(parent->children, index);
        this->parent = NULL;

        if (linked_list_count(parent->children) == 0) {
            free(parent->children);
            parent->children = NULL;
        }
    }
}

void fctx_layer_mark_dirty(const FctxLayer *this) {
    logf();
    layer_mark_dirty(this->layer);
}

bool fctx_layer_get_hidden(const FctxLayer *this) {
    logf();
    return layer_get_hidden(this->layer);
}

void fctx_layer_set_hidden(FctxLayer *this, bool hidden) {
    logf();
    layer_set_hidden(this->layer, hidden);
}

GRect fctx_layer_get_frame(const FctxLayer *this) {
    logf();
    return layer_get_frame(this->layer);
}

void fctx_layer_set_frame(FctxLayer *this, GRect frame) {
    logf();
    layer_set_frame(this->layer, frame);
}

GRect fctx_layer_get_bounds(const FctxLayer *this) {
    logf();
    return layer_get_bounds(this->layer);
}

void fctx_layer_set_bounds(FctxLayer *this, GRect bounds) {
    logf();
    layer_set_bounds(this->layer, bounds);
}

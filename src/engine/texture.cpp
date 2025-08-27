#include "texture.h"
#include "engine.h"
#include "exceptions.h"


Texture::Texture(SDL_Surface* s, const int w, const int h) {
    SDL_Texture* new_texture = SDL_CreateTextureFromSurface(gRenderer, s);
    if (new_texture == nullptr) {
        throw image_load_exception(std::string(SDL_GetError()));
    }
    this->width = w;
    this->height = h;
    this->texture = new_texture;
}

void Texture::load_from_file(const std::string& path) {
	free();
	
	SDL_Texture* new_texture = IMG_LoadTexture(gRenderer, path.c_str());
	
	if (new_texture == nullptr) {
		throw image_load_exception(std::string(SDL_GetError()));
	}
	// Use QueryTexture to set width and height.
        width = texture->w;
        height = texture->h;
	texture = new_texture;
}

void Texture::load_from_file(const std::string& path, const int w, const int h) {
	free();
	SDL_Surface* surface = IMG_Load(path.c_str());
	if (surface == nullptr) {
            throw image_load_exception(std::string(SDL_GetError()));
	}
	SDL_PixelFormat format = surface->format;
	SDL_Surface* stretched = SDL_CreateSurface(w, h, format);
	if (stretched == nullptr) {
            SDL_DestroySurface(surface);
            throw image_load_exception(std::string(SDL_GetError()));
	}
	SDL_BlitSurfaceScaled(surface, nullptr, stretched, nullptr,
                              SDL_SCALEMODE_NEAREST);
	SDL_DestroySurface(surface);
	
	texture = SDL_CreateTextureFromSurface(gRenderer, stretched);
	SDL_DestroySurface(stretched);
	
	if (texture == nullptr) {
            throw image_load_exception(std::string(SDL_GetError()));
	}
	
	width = w;
	height = h;	
}

void Texture::load_sub_image(SDL_Surface *src, SDL_Rect source_rect, int w, int h) {
    free();

    SDL_PixelFormat format = src->format;
    SDL_Surface* stretched = SDL_CreateSurface(w, h, format);
    if (stretched == nullptr) {
        throw image_load_exception(std::string(SDL_GetError()));
    }
    SDL_BlitSurfaceScaled(src, &source_rect, stretched, nullptr,
                          SDL_SCALEMODE_NEAREST);
    texture = SDL_CreateTextureFromSurface(gRenderer, stretched);
    SDL_DestroySurface(stretched);

    if (texture == nullptr) {
        throw image_load_exception(std::string(SDL_GetError()));
    }

    width = w;
    height = h;
}

void Texture::load_sub_image(SDL_Surface *src, SDL_Rect source_rect) {
    load_sub_image(src, source_rect,  source_rect.w, source_rect.h);
}


void Texture::free() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

void Texture::render_corner(int x, int y) const {
    SDL_FRect rect = {(float)x, (float)y, (float)width, (float)height};
    SDL_RenderTexture(gRenderer, texture, nullptr, &rect);
}

void Texture::render(const int x, const int y) const {
    SDL_FRect rect = {(float)(x - width / 2.0f),
                      (float)(y - height / 2.0f),
                      (float)width, (float)height};
    SDL_RenderTexture(gRenderer, texture, nullptr, &rect);
}

void Texture::render(const int x, const int y, const double angle) const {
    SDL_FRect rect = {(float)(x - width / 2.0f),
                      (float)(y - height / 2.0f), 
                      (float)width, (float)height};
    SDL_RenderTextureRotated(
        gRenderer,
        texture,
        nullptr,
        &rect,
        angle * 180 / 3.14159265,
        nullptr,
        SDL_FLIP_NONE);
}

void Texture::render(const int x, const int y, const double angle, 
                     const SDL_FlipMode flip) const {
    SDL_FRect rect = {(float)(x - width / 2.0f),
                      (float)(y - height / 2.0f),
                      (float)width, (float)height};
    SDL_RenderTextureRotated(
        gRenderer,
        texture,
        nullptr,
        &rect,
        angle * 180 / 3.14159265,
        nullptr,
        flip);
}

void Texture::render_corner_f(float x, float y, float w, float h, 
                              double angle, SDL_FlipMode flip) const {
    SDL_FRect dest = {x, y, w, h};
    SDL_RenderTextureRotated(gRenderer, texture, nullptr, &dest, 
                             angle, nullptr, flip);
}

void Texture::render(const int dest_x, const int dest_y, const int x, const int y, const int w, const int h) const {
    SDL_FRect target = {(float)dest_x, (float)dest_y,
                        (float)width, (float)height};
    SDL_FRect source = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderTexture(gRenderer, texture, &source, &target);
}

int Texture::get_height() const {
    return height;
}

int Texture::get_width() const {
    return width;
}

void Texture::set_dimensions(const int w, const int h) {
    this->width = w;
    this->height = h;
}

void Texture::set_color_mod(Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetTextureColorMod(texture, r, g, b);
}

Texture::~Texture() {
    free();
}

Texture::Texture(Texture&& o) noexcept {
    free();
    texture = o.texture;
    width = o.width;
    height = o.height;
    o.texture = nullptr;
}

Texture& Texture::operator=(Texture&& o) noexcept {
	free();
	texture = o.texture;
	width = o.width;
	height = o.height;
	o.texture = nullptr;
	return *this;
}

PositionedTexture::PositionedTexture(const Texture *texture, int x, int y) : texture(texture), x(x), y(y),
                                            r(255), g(255), b(255) {}

void PositionedTexture::render() {
    const_cast<Texture*>(texture)->set_color_mod(r, g, b);
    texture->render_corner(x, y);
}

int PositionedTexture::index() {
    return y + texture->get_height();
}

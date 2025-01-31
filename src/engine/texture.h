#ifndef TEXTURE_H_00
#define TEXTURE_H_00

#include <SDL.h>
#include <SDL_image.h>
#include <string>

/**
 * Wrapper class for an SDL_Texture, also containing width and height.
 */
class Texture {
    public:
        Texture() = default;

        Texture(SDL_Texture* const t, const int w, const int h) : texture(t), width(w), height(h) {};

        Texture(SDL_Surface* s, int w, int h);

        /**
         * Copying a texture is not supported, since it would be very rarely desired.
         * If so, copy the underlying SDL_Texture and create a new Texture instead.
	 */
        Texture(const Texture& o) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& o) noexcept;

        Texture& operator=(Texture&&) noexcept;

        ~Texture();
		
        /**
	 * Reads a texture from a file, trowing a image_load_exception if something goes wrong.
	 */
        void load_from_file(const std::string& path);

        /**
         * Reads a texture form a file and stretches it to (w, h), throwing a image_load_exception if something goes wrong.
         */
        void load_from_file(const std::string& path, int w, int h);

        void load_sub_image(SDL_Surface* src, SDL_Rect source_rect, int w, int h);

        void load_sub_image(SDL_Surface *src, SDL_Rect source_rect);

        /**
	 * Frees all resources associated with this texture.
	 */
        void free();

        void render_corner(int x, int y) const;

        void render_corner_f(float x, float y, float w, float h, double angle, SDL_RendererFlip flip) const;
		
        /**
	 * Renders this texture at position (x, y) using the global gRenderer.
	 */
        void render(int x, int y) const;
		
        /**
	 * Renders the rectangle (x, y, w, h) of this texture at position (dest_x, dest_y),
	 * using the global gRenderer.
	 */
        void render(int dest_x, int dest_y, int x, int y, int w, int h) const;

        /**
         * Render this texture at position (x, y) rotated by angle using the global gRenderer.
         */
        void render(int x, int y, double angle) const;

        /**
         * Render this texture at position (x, y) rotated by angle and flipped according to flip.
         */
        void render(int x, int y, double angle, SDL_RendererFlip flip) const;
		
        /**
	 * Returns the width of this texture.
	*/
        [[nodiscard]] int get_width() const;
		
        /**
	 * Returns the height of this texture.
	 */
        [[nodiscard]] int get_height() const;
		
        /**
	 * Sets the width and height.
	 */
        void set_dimensions(int w, int h);
		
        /**
	 * Sets the color modulation.
	 */
        void set_color_mod(Uint8 r, Uint8 g, Uint8 b);
    private:
        SDL_Texture* texture = nullptr;
        int width = 0;
        int height = 0;
};

class SortableTexture {
public:
    virtual void render() = 0;

    virtual int index() = 0;

    virtual ~SortableTexture() = default;
};

struct PositionedTexture : public SortableTexture {
    PositionedTexture(const Texture* texture, int x, int y);

    void render() override;
    int index() override;

    const Texture* texture;
    int x, y;
    Uint8 r, g, b;
};
#endif
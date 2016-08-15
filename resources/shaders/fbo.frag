#version 330

#define BLUR_KERNAL_SIZE        5
#define BLUR_KERNAL_STRIDE      1
#define OUTLINE_COLOUR          0.8, 1.0, 1.0

in vec2 texCoordOut;

uniform sampler2D uTextureUnit0;
uniform isampler2D uTextureUnit1;

out vec4 colourOutput;

vec4 getBlurredPixel(in ivec3 outline)
{
    vec2 dim = textureSize(uTextureUnit1, 0);

    int outlineCount = 0;
    int sampleCount  = 0;

    for (int i = -(BLUR_KERNAL_SIZE-1); i < BLUR_KERNAL_SIZE; i+=BLUR_KERNAL_STRIDE)
    {
        for (int j = -(BLUR_KERNAL_SIZE-1); j < BLUR_KERNAL_SIZE; j+=BLUR_KERNAL_STRIDE)
        {
            float x = texCoordOut.x + (i / dim.x);
            float y = texCoordOut.y + (j / dim.y);

            ivec4 value = texture(uTextureUnit1, vec2(x, y));

            if (value.rgb == outline.rgb)
                outlineCount += value.a;

            sampleCount++;
        }
    }

    float x = outlineCount / float(sampleCount);

    if (outlineCount == sampleCount)
        return vec4(0, 0, 0, 0);

    return vec4(OUTLINE_COLOUR, (1 - x) * pow(x, 0.5));
}


void main()
{
    // get outline buffer colour
    vec4 colour = texture(uTextureUnit0, vec2(texCoordOut.x, texCoordOut.y));
    ivec4 outline = texture(uTextureUnit1, vec2(texCoordOut.x, texCoordOut.y));

    // if outline is clear, just render
    if (outline.a == 0)
    {
        colourOutput = colour;
        return;
    }

    // otherwise blur the outline
    vec4 blur = getBlurredPixel(outline.rgb);
    colourOutput = mix(colour, blur, blur.a) + vec4(blur.rgb * blur.a, blur.a);
}

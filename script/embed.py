import sys
from PIL import Image

with Image.open(sys.argv[1]) as image:
    outfile = open(sys.argv[2], "w")

    for y in range(image.size[1]):
        for x in range(image.size[0]):
            pixel = image.getpixel((x, y))
            r, g, b, a = pixel
            pixel_data = (a << 24) | (b << 16) | (g << 8) | r
            outfile.write(hex(pixel_data) + ",")
        outfile.write("\n")
    
    outfile.close()
    

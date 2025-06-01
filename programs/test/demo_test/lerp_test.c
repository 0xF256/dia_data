#include <stdio.h>

int main() {
    int startX, startY, endX, endY, frames;

    printf("Please enter startX startY endX endY frames\n");
    scanf("%d %d %d %d %d", &startX, &startY, &endX, &endY, &frames);

    if (frames <= 0)
    {
        printf("Err: frames must be positive\n");
        return 1;
    }

    for(int frame = 0; frame <= frames; frame++)
    {
        int offsetX = (endX * frame + startX * (frames - frame)) / frames;
        int offsetY = (endY * frame + startY * (frames - frame)) / frames;

        printf("Frame%02d: Pos(%d, %d)\n", frame, offsetX, offsetY);
    }

    return 0;
}
#include <windows.h>
#include <GL/gl.h>
#include "glext.h"
#include "config.h"
#include "stb_image_write.h"

#define NUM_THREADS 8
#define QUEUE_SIZE 8
#define FRAME_SIZE 3*XRES*YRES

/**
 * Almost textbook producer/consumer implementation:
 * https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
 * Generating a frame is usually faster than converting it to a PNG and saving
 * it on the disk. Thus the main thread captures frames in a buffer while
 * multiple threads save the buffered frames in parallel.
 */

typedef struct frame_t
{
    int id;
    unsigned char pixels[FRAME_SIZE];
} frame_t;

typedef struct frame_queue_t
{
    int next;
    int last;
    HANDLE free_slots_sem;
    HANDLE full_slots_sem;
    HANDLE mutex;
    frame_t frames[QUEUE_SIZE];
} frame_queue_t;

static frame_queue_t frame_queue;

static HANDLE threads[NUM_THREADS];
static DWORD thread_ids[NUM_THREADS];

static BOOL should_terminate = FALSE;


DWORD WINAPI save_queued_frames();

void init_capture(HWND hwnd) {
    CreateDirectory("capture", NULL);

    #ifdef VIDEO
    frame_queue.next = frame_queue.last = 0;
    frame_queue.free_slots_sem = CreateSemaphore(NULL, QUEUE_SIZE, QUEUE_SIZE, NULL);
    frame_queue.full_slots_sem = CreateSemaphore(NULL, 0, QUEUE_SIZE, NULL);
    frame_queue.mutex = CreateMutex(NULL, FALSE, NULL);

    for(int i = 0; i < NUM_THREADS; i++) {
        threads[i] = CreateThread(NULL, 0, save_queued_frames, NULL, 0, &thread_ids[i]);
        if(threads[i] == NULL) {
            MessageBox(hwnd, "Failed to create thread.", "Error", MB_OK);
            ExitProcess(-1);
        }
    }

    stbi_flip_vertically_on_write(1);

    glReadBuffer(GL_FRONT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    #endif
}

void end_capture(HWND hwnd) {
    #ifdef VIDEO
    should_terminate = TRUE;
    // signal each thread to terminate
    // since NUM_THREADS and QUEUE_SIZE could be different, we need to manually
    // signal each thread and wait for it to end before signaling the next one
    for(int i = 0; i < NUM_THREADS; i++) {
        ReleaseSemaphore(frame_queue.full_slots_sem, 1, NULL);
        WaitForMultipleObjects(NUM_THREADS, threads, FALSE, INFINITE);
    }

    for(int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }

    CloseHandle(frame_queue.free_slots_sem);
    CloseHandle(frame_queue.full_slots_sem);
    CloseHandle(frame_queue.mutex);
    #endif
}

DWORD WINAPI save_queued_frames() {
    // consumer
    char filename[32];
    while(1) {
        WaitForSingleObject(frame_queue.full_slots_sem, INFINITE);
        if(should_terminate) break;

        WaitForSingleObject(frame_queue.mutex, INFINITE);
        int frame_index = frame_queue.last;
        frame_queue.last = (frame_queue.last + 1) % QUEUE_SIZE;
        ReleaseMutex(frame_queue.mutex);

        frame_t* frame = &frame_queue.frames[frame_index];
        wsprintf(filename, ".\\capture\\frame_%06d.png", frame->id);
        stbi_write_png(filename, XRES, YRES, 3, frame->pixels, 3*XRES);

        ReleaseSemaphore(frame_queue.free_slots_sem, 1, NULL);
    }
    return 0;
}

void save_frame(int frame_id) {
    // producer
    WaitForSingleObject(frame_queue.free_slots_sem, INFINITE);
    
    frame_t* frame = &frame_queue.frames[frame_queue.next];
    frame->id = frame_id;
    glReadPixels(0, 0, XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, frame->pixels);
    frame_queue.next = (frame_queue.next + 1) % QUEUE_SIZE;

    ReleaseSemaphore(frame_queue.full_slots_sem, 1, NULL);
}

void save_audio(short* buffer, int bytes, HWND hwnd) {
    HANDLE file = CreateFile(
        ".\\capture\\audio.raw",
        GENERIC_WRITE,
        0,
        NULL, 
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if(file == INVALID_HANDLE_VALUE) {
        MessageBox(hwnd, "Failed to create audio file.", "Error", MB_OK);
        ExitProcess(-1);
        return;
    }

    DWORD nbWritten;
    if(!WriteFile(file, (LPCVOID)buffer, (DWORD)bytes, &nbWritten, NULL)) {
        MessageBox(hwnd, "Failed to write audio file.", "Error", MB_OK);
        ExitProcess(-1);
    }
}


# Jessi Lab Tuto Kamehameha

import pygame
import serial
import time
import math
import random
import sys

PORT = "COM4"
BAUD = 115200

FPS = 60
CHARGE_DURATION = 1.0
EXPLOSION_DURATION = 1.6

pygame.init()

screen = pygame.display.set_mode(
    (0, 0),
    pygame.FULLSCREEN
)

W, H = screen.get_size()
CX, CY = W // 2, H // 2

pygame.display.set_caption("Jessi Lab - Kamehameha")

clock = pygame.time.Clock()

IDLE = "IDLE"
CHARGE = "CHARGE"
FIRE = "FIRE"

state = IDLE

charge_start = 0.0
fire_start = 0.0

serial_buffer = bytearray()
arduino = None

try:
    arduino = serial.Serial(PORT, BAUD, timeout=0)
    time.sleep(2)
    arduino.reset_input_buffer()
    print("Arduino connecte sur", PORT)

except serial.SerialException as error:
    print("Connexion impossible :", error)
    print("Mode clavier disponible.")

MAX_RADIUS = int(min(W * 0.17, H * 0.28))

rng = random.Random(1234)

stars = [
    (
        rng.randrange(W),
        rng.randrange(H),
        rng.randrange(40, 130),
        rng.choice([1, 1, 1, 2])
    )
    for _ in range(90)
]

effects = pygame.Surface(
    (W, H),
    pygame.SRCALPHA
)

flash_surface = pygame.Surface(
    (W, H),
    pygame.SRCALPHA
)

def clamp(value, minimum=0.0, maximum=1.0):
    return max(minimum, min(maximum, value))


def ease_out(value):
    value = clamp(value)
    return 1 - (1 - value) ** 3


def draw_background():
    screen.fill((2, 4, 13))

    for x, y, brightness, radius in stars:
        pygame.draw.circle(
            screen,
            (
                brightness // 3,
                brightness // 2,
                brightness
            ),
            (x, y),
            radius
        )


def draw_glow(x, y, radius, strength=1.0):
    radius = max(1, int(radius))

    for scale, alpha in [
        (2.5, 8),
        (2.0, 13),
        (1.65, 20),
        (1.35, 30),
        (1.15, 42)
    ]:
        pygame.draw.circle(
            effects,
            (
                0,
                115,
                255,
                int(alpha * strength)
            ),
            (int(x), int(y)),
            int(radius * scale)
        )


def draw_energy_ball(now, progress):
    progress = ease_out(progress)

    radius = (
        15 +
        (MAX_RADIUS - 15) * progress
    )

    pulse = 1 + 0.035 * math.sin(now * 11)

    radius *= pulse
    radius = int(radius)

    draw_glow(CX, CY, radius)

    for i in range(3):
        angle = now * (1.1 + i * 0.3) + i * 2

        rx = radius * (1.35 + i * 0.15)
        ry = radius * (0.45 + i * 0.1)

        points = []

        for j in range(45):
            a = j / 44 * math.tau

            px = math.cos(a) * rx
            py = math.sin(a) * ry

            rotated_x = (
                px * math.cos(angle) -
                py * math.sin(angle)
            )

            rotated_y = (
                px * math.sin(angle) +
                py * math.cos(angle)
            )

            points.append((
                int(CX + rotated_x),
                int(CY + rotated_y)
            ))

        pygame.draw.lines(
            effects,
            (30, 160, 255, 55),
            True,
            points,
            2
        )

    for i in range(70):
        angle = i * 2.399963

        phase = (
            now * 0.85 +
            i * 0.137
        ) % 1.0

        distance = (
            radius * 1.15 +
            (1 - phase) * 240
        )

        px = CX + math.cos(angle) * distance
        py = CY + math.sin(angle) * distance

        particle_radius = 1 if phase < 0.5 else 2

        pygame.draw.circle(
            effects,
            (65, 195, 255, 170),
            (int(px), int(py)),
            particle_radius
        )

    pygame.draw.circle(
        effects,
        (0, 70, 220, 255),
        (CX, CY),
        radius
    )

    pygame.draw.circle(
        effects,
        (0, 170, 255, 255),
        (CX, CY),
        int(radius * 0.78)
    )

    pygame.draw.circle(
        effects,
        (125, 230, 255, 255),
        (CX, CY),
        int(radius * 0.53)
    )

    pygame.draw.circle(
        effects,
        (245, 252, 255, 255),
        (CX, CY),
        max(2, int(radius * 0.27))
    )


explosion_particles = []


def create_explosion_particles():
    global explosion_particles

    explosion_particles = []

    for i in range(180):
        angle = random.uniform(0, math.tau)
        speed = random.uniform(180, 950)

        explosion_particles.append({
            "angle": angle,
            "speed": speed,
            "size": random.randint(1, 5),
            "delay": random.uniform(0, 0.15)
        })


def draw_explosion(now, elapsed):
    expansion = ease_out(elapsed / 0.38)

    if elapsed < 0.22:
        flash = 1 - elapsed / 0.22
        alpha = int(220 * flash)

        flash_surface.fill(
            (210, 240, 255, alpha)
        )

        screen.blit(
            flash_surface,
            (0, 0)
        )

    if elapsed < 0.9:
        shock_progress = ease_out(
            elapsed / 0.9
        )

        shock_radius = int(
            15 +
            max(W, H) * 0.75 * shock_progress
        )

        alpha = int(
            220 * (1 - shock_progress)
        )

        pygame.draw.circle(
            effects,
            (0, 170, 255, alpha // 3),
            (CX, CY),
            shock_radius,
            max(1, int(35 * (1 - shock_progress)))
        )

        pygame.draw.circle(
            effects,
            (180, 240, 255, alpha),
            (CX, CY),
            shock_radius,
            max(1, int(9 * (1 - shock_progress)))
        )

    for particle in explosion_particles:
        t = elapsed - particle["delay"]

        if t < 0:
            continue

        life = clamp(1 - t / 1.25)

        if life <= 0:
            continue

        distance = particle["speed"] * t
        angle = particle["angle"]

        px = CX + math.cos(angle) * distance
        py = CY + math.sin(angle) * distance

        if not (0 <= px < W and 0 <= py < H):
            continue

        alpha = int(230 * life)

        pygame.draw.circle(
            effects,
            (65, 190, 255, alpha),
            (int(px), int(py)),
            particle["size"]
        )

    if elapsed < 0.35:
        intensity = 1 - elapsed / 0.35

        core_radius = int(
            MAX_RADIUS * (1 + 2.5 * expansion)
        )

        draw_glow(
            CX,
            CY,
            core_radius,
            intensity
        )

        pygame.draw.circle(
            effects,
            (
                190,
                240,
                255,
                int(220 * intensity)
            ),
            (CX, CY),
            core_radius
        )

    return elapsed >= EXPLOSION_DURATION


def start_charge():
    global state, charge_start

    if state != IDLE:
        return

    state = CHARGE
    charge_start = time.monotonic()

    print("CHARGE")


def start_fire():
    global state, fire_start

    if state != CHARGE:
        return

    state = FIRE
    fire_start = time.monotonic()

    create_explosion_particles()

    print("FIRE")


def reset():
    global state

    if state == CHARGE:
        state = IDLE
        print("RESET")


def process_command(message):
    if message == "CHARGE":
        start_charge()

    elif message == "FIRE":
        start_fire()

    elif message == "RESET":
        reset()


running = True

while running:
    clock.tick(FPS)
    now = time.monotonic()

    if arduino is not None:
        try:
            available = arduino.in_waiting

            if available:
                serial_buffer.extend(
                    arduino.read(available)
                )

                while b"\n" in serial_buffer:
                    raw, _, rest = (
                        serial_buffer.partition(b"\n")
                    )

                    serial_buffer = bytearray(rest)

                    message = raw.decode(
                        "utf-8",
                        errors="ignore"
                    ).strip()

                    if message:
                        print("ARDUINO ->", message)
                        process_command(message)

                if len(serial_buffer) > 512:
                    serial_buffer.clear()

        except serial.SerialException as error:
            print("Erreur serie :", error)

            try:
                arduino.close()
            except Exception:
                pass

            arduino = None

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False

            elif event.key == pygame.K_c:
                start_charge()

            elif event.key == pygame.K_SPACE:
                start_fire()

            elif event.key == pygame.K_r:
                reset()

    draw_background()
    effects.fill((0, 0, 0, 0))

    if state == CHARGE:
        progress = clamp(
            (now - charge_start) /
            CHARGE_DURATION
        )

        draw_energy_ball(now, progress)

    elif state == FIRE:
        finished = draw_explosion(
            now,
            now - fire_start
        )

        if finished:
            state = IDLE

    screen.blit(
        effects,
        (0, 0)
    )

    pygame.display.flip()

if arduino is not None:
    arduino.close()

pygame.quit()
sys.exit()

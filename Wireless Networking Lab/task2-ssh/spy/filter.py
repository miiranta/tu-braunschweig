import pygame
import re
import sys
import select
import random
from termcolor import colored

# Initialize Pygame
pygame.init()

# Constants
GRID_SIZE = 5
CELL_SIZE = 100
MARGIN = CELL_SIZE  # Margin size equal to one cell size
WIDTH = (GRID_SIZE + 2) * CELL_SIZE  # Adding margins on both sides
HEIGHT = (GRID_SIZE + 2) * CELL_SIZE  # Adding margins on both sides
LINE_WIDTH = 2
LINE_COLOR = (0, 0, 0)  # Black
CAR_IMAGE_PATH = 'car.png'  # Path to the car image

# Predefined colors
PREDEFINED_COLORS = [
    (255, 0, 0),                    # Red
    (0, 255, 0),                    # Green
    (0, 0, 255),                    # Blue
    (255, 255, 0),                  # Yellow
    (255, 0, 255),                  # Magenta
    (0, 255, 255),                  # Cyan
    (192, 192, 192),                # Silver
    (128, 0, 0),                    # Maroon
    (128, 128, 0),                  # Olive
    (0, 128, 0),                    # Dark Green
    (128, 0, 128),                  # Purple
    (0, 128, 128),                  # Teal
    (0, 0, 128),                    # Navy
    (255, 165, 0),                  # Orange
    (255, 192, 203),                # Pink
    (105, 105, 105)                 # Dim Gray
]

# Set up the display
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Virtual Maze Grid Visualization")

# Regex patterns
primary_pattern = re.compile(r'# RECEIVED PACKET: (.+)')
handshake_pattern = re.compile(r'^H.*(\d)$')  # Capture the last digit after 'H'
movement_pattern = re.compile(r'^L.{2}C(\d)$')  # Match 'L' followed by 2 chars, 'C', and a digit
new_car_pattern = re.compile(r'^C.*(\d)$')  # Match 'C', followed by any characters, and capture the final digit as car ID

# Dictionary to store car images by ID
car_images = {}
# Dictionary to store car positions, orientations, colors, and offsets by ID
car_data = {}

# Load the car image
def load_car_image(car_id):
    """Load and return a car image based on its ID."""
    if car_id not in car_images:
        # Load the original car image
        original_image = pygame.image.load(CAR_IMAGE_PATH)

        # Scale to fit the grid cell size
        width, height = original_image.get_size()
        aspect_ratio = width / height
        new_width = CELL_SIZE
        new_height = int(CELL_SIZE / aspect_ratio)

        # If the new height exceeds cell size, adjust width instead
        if new_height > CELL_SIZE:
            new_height = CELL_SIZE
            new_width = int(CELL_SIZE * aspect_ratio)

        # Resize the image with new dimensions
        resized_image = pygame.transform.scale(original_image, (new_width, new_height))

        # Create a surface with transparency
        car_surface = pygame.Surface((CELL_SIZE, CELL_SIZE), pygame.SRCALPHA)
        # Center the resized image on the surface
        x = (CELL_SIZE - new_width) // 2
        y = (CELL_SIZE - new_height) // 2
        car_surface.blit(resized_image, (x, y))
        
        car_images[car_id] = car_surface

    return car_images[car_id]

def rotate_car_image(image, angle):
    """Rotate the car image by a given angle."""
    return pygame.transform.rotate(image, angle)

def get_unique_color():
    """Get a unique color from predefined colors."""
    if not PREDEFINED_COLORS:
        raise Exception("No more colors available.")
    return PREDEFINED_COLORS.pop(random.randint(0, len(PREDEFINED_COLORS) - 1))

def draw_grid():
    """Draw the grid with margins in bottom-left orientation."""
    # Fill the background with white
    screen.fill((255, 255, 255))  # White

    # Draw vertical lines
    for x in range(MARGIN, WIDTH - MARGIN + CELL_SIZE, CELL_SIZE):
        pygame.draw.line(screen, LINE_COLOR, (x, HEIGHT - MARGIN), (x, MARGIN), LINE_WIDTH)

    # Draw horizontal lines
    for y in range(MARGIN, HEIGHT - MARGIN + CELL_SIZE, CELL_SIZE):
        pygame.draw.line(screen, LINE_COLOR, (MARGIN, HEIGHT - y), (WIDTH - MARGIN, HEIGHT - y), LINE_WIDTH)

    # Draw the goal marker at (3, 3)
    goal_x = (3 + 1) * CELL_SIZE
    goal_y = HEIGHT - (3 + 1) * CELL_SIZE
    pygame.draw.circle(screen, (255, 0, 0), (goal_x, goal_y), 10)  # Red 
    
    # Draw the begin marker at (0, 0)
    begin_x = (0 + 1) * CELL_SIZE
    begin_y = HEIGHT - (0 + 1) * CELL_SIZE
    pygame.draw.circle(screen, (0, 255, 0), (begin_x, begin_y), 10)  # Green

offsets = []
for offset_x in [-5, 0, 5]:
    for offset_y in [-5, 0, 5]:
        offsets.append((offset_x, offset_y))

def draw_cars():
    """Draw all cars based on their positions, orientations, and colors."""
    for car_id, (pos, orientation, color, offset_index) in car_data.items():
        car_image = load_car_image(car_id)
        rotated_image = rotate_car_image(car_image, -90 * orientation)  # Rotate car image based on orientation

        # Apply the assigned offset to the car's position
        offset = offsets[offset_index]
        x, y = pos
        x -= CELL_SIZE // 2
        y += CELL_SIZE // 2

        # Create a surface with the car image and fill it with the car's color
        car_surface = pygame.Surface((CELL_SIZE, CELL_SIZE), pygame.SRCALPHA)
        car_surface.blit(rotated_image, (0, 0))
        car_surface.fill(color, special_flags=pygame.BLEND_MULT)
        
        screen.blit(car_surface, (x + offset[0], y + offset[1]))  # Draw car at its position

def create_if_not_exists(car_id):
    """Create a new car if it doesn't exist in the car data."""
    if car_id not in car_data:
        offset_index = len(car_data) % len(offsets)  # Assign an offset index based on car count
        # Initialize car with default position, orientation, and a unique color
        car_data[car_id] = ((MARGIN, HEIGHT - CELL_SIZE - MARGIN), 0, get_unique_color(), offset_index)
    
        return f'New Car ID {car_id} initialized', 'white'

    return '', ''

def process_message(message):
    """Process and filter incoming messages using regex."""
    match = primary_pattern.search(message)
    if match:
        data = match.group(1).strip()

        # Process specific message types
        handshake_match = handshake_pattern.match(data)
        if handshake_match:
            new_car_id = handshake_match.group(1)
            return create_if_not_exists(new_car_id)

        movement_match = movement_pattern.match(data)
        if movement_match:
            car_id = data[1]
            direction = movement_match.group(1)
            if car_id in car_data:
                (x, y), orientation, color, offset_index = car_data[car_id]
                
                # Move car and update orientation based on direction
                if direction == '1':
                    orientation = (orientation + 1) % 4
                elif direction == '2':
                    orientation = (orientation - 1) % 4
                elif direction == '3':
                    pass

                if orientation == 0:    # Facing up
                    y -= CELL_SIZE
                elif orientation == 1:  # Facing right
                    x += CELL_SIZE
                elif orientation == 2:  # Facing down
                    y += CELL_SIZE
                elif orientation == 3:  # Facing left
                    x -= CELL_SIZE
                
                # Update car data with new position, orientation, and color
                car_data[car_id] = ((x, y), orientation, color, offset_index)
            return f'Car ID {car_id} in ({x}, {y})', 'white'

        new_car_match = new_car_pattern.match(data)
        if new_car_match:
            new_car_id = new_car_match.group(1)
            return create_if_not_exists(new_car_id)

        return data, 'white'  # Default color if no specific pattern matched

    return message, 'green'  # Color for non-matching messages

def main():
    clock = pygame.time.Clock()
    stdin_fd = sys.stdin.fileno()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        # Use select to check for input availability
        readable, _, _ = select.select([stdin_fd], [], [], 0.1)
        if readable:
            message = sys.stdin.readline().strip()
            if not message:
                continue  # Skip if no message

            draw_grid()
            processed_message, color = process_message(message)

            draw_cars()

            if color == 'white':
                print(colored(processed_message, 'white'))
            elif color == 'green':
                print(colored(processed_message, 'green'))
            
            pygame.display.flip()
        
        clock.tick(30)  # Limit to 30 frames per second

if __name__ == "__main__":
    main()

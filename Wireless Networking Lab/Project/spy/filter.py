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
TEXT_AREA_WIDTH = 300  # Width of the text area
WIDTH = (GRID_SIZE + 2) * CELL_SIZE + TEXT_AREA_WIDTH  # Adjust the width
HEIGHT = (GRID_SIZE + 2) * CELL_SIZE  # Adding margins on both sides
LINE_WIDTH = 2
LINE_COLOR = (0, 0, 0)  # Black
CAR_IMAGE_PATH = 'car.png'  # Path to the car image

# Button constants
BUTTON_WIDTH = 150
BUTTON_HEIGHT = 50
BUTTON_COLOR = (200, 200, 200)  # Light gray
BUTTON_HOVER_COLOR = (150, 150, 150)  # Darker gray
BUTTON_TEXT_COLOR = (0, 0, 0)  # Black
RESET_BUTTON_RECT = pygame.Rect(
    (TEXT_AREA_WIDTH - BUTTON_WIDTH) // 2 + (GRID_SIZE + 2) * CELL_SIZE + MARGIN // 2,  # Centered horizontally
    HEIGHT - BUTTON_HEIGHT - 20,  # Positioned 20 pixels from the bottom of the text area
    BUTTON_WIDTH,
    BUTTON_HEIGHT
)

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
available_colors = PREDEFINED_COLORS.copy()

# Set up the display
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Virtual Maze Grid Visualization")

# Regex patterns
primary_pattern = re.compile(r'# RECEIVED PACKET: (.+)')
handshake_pattern = re.compile(r'^H.*(\d)$')  # Capture the last digit after 'H'
movement_pattern = re.compile(r'^L.{2}C(\d)$')  # Match 'L' followed by 2 chars, 'C', and a digit
new_car_pattern = re.compile(r'^C.*(\d)$')  # Match 'C', followed by any characters, and capture the final digit as car ID
path_pattern = re.compile(r'^L(\d+)\wP(\d+)$')  # Match 'L' followed by car ID, digits, 'P', and a string of numbers

# Dictionary to store car images by ID
car_images = {}
# Dictionary to store car positions, orientations, colors, and offsets by ID
car_data = {}
# Dictionary to store car paths by ID
car_paths = {}

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
    if not available_colors:
        raise Exception("No more colors available.")
    return available_colors.pop(random.randint(0, len(available_colors) - 1))

def draw_grid():
    """Draw the grid with margins in bottom-left orientation."""
    # Fill the background with white
    screen.fill((255, 255, 255))  # White

    # Draw vertical lines
    for x in range(MARGIN, WIDTH - MARGIN - TEXT_AREA_WIDTH + CELL_SIZE, CELL_SIZE):
        pygame.draw.line(screen, LINE_COLOR, (x, HEIGHT - MARGIN), (x, MARGIN), LINE_WIDTH)

    # Draw horizontal lines
    for y in range(MARGIN, HEIGHT - MARGIN + CELL_SIZE, CELL_SIZE):
        pygame.draw.line(screen, LINE_COLOR, (MARGIN, HEIGHT - y), (WIDTH - MARGIN - TEXT_AREA_WIDTH, HEIGHT - y), LINE_WIDTH)

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

def draw_paths():
    """Draw the paths for all cars."""
    for car_id, path in car_paths.items():
        if path:
            color = car_data[car_id][2]  # Get the car's color
            offset = offsets[car_data[car_id][3]]  # Get the car's offset

            for i in range(len(path) - 1):
                start_pos = (path[i][0] + offset[0], path[i][1] + offset[1])
                end_pos = (path[i + 1][0] + offset[0], path[i + 1][1] + offset[1])
                pygame.draw.line(screen, color, start_pos, end_pos, LINE_WIDTH)

def draw_text_area():
    """Draw the text area with car information and path sizes."""
    font = pygame.font.SysFont(None, 24)
    x_start = (GRID_SIZE + 2) * CELL_SIZE + MARGIN // 2

    # Draw the background of the text area
    pygame.draw.rect(screen, (240, 240, 240), (x_start, 0, TEXT_AREA_WIDTH, HEIGHT))

    y = MARGIN
    for car_id, (pos, orientation, color, offset_index) in car_data.items():
        # Render car ID
        text = f"Car ID: {car_id}"
        img = font.render(text, True, color)
        screen.blit(img, (x_start + 10, y))

        # Render path size
        path_size = len(car_paths.get(car_id, [])) - 1  # Subtract 1 to get the number of segments
        path_size_text = f"Path Size: {path_size}"
        path_size_img = font.render(path_size_text, True, color)
        screen.blit(path_size_img, (x_start + 10, y + 30))

        y += 60  # Adjust for the additional line

    draw_button(x_start)  # Draw the reset button

def draw_button(x_start):
    """Draw the reset button in the text area."""
    mouse_x, mouse_y = pygame.mouse.get_pos()
    is_hovered = RESET_BUTTON_RECT.collidepoint(mouse_x, mouse_y)
    button_color = BUTTON_HOVER_COLOR if is_hovered else BUTTON_COLOR

    # Draw button background
    pygame.draw.rect(screen, button_color, RESET_BUTTON_RECT)

    # Draw button text
    font = pygame.font.SysFont('Arial', 24)  # Use 'Arial' to avoid system font issues
    text = font.render('Reset', True, BUTTON_TEXT_COLOR)
    text_rect = text.get_rect(center=RESET_BUTTON_RECT.center)
    screen.blit(text, text_rect)
    
def create_if_not_exists(car_id):
    """Create a new car if it doesn't exist in the car data."""
    if car_id not in car_data:
        offset_index = len(car_data) % len(offsets)  # Assign an offset index based on car count
        # Initialize car with default position, orientation, and a unique color
        car_data[car_id] = ((MARGIN, HEIGHT - CELL_SIZE - MARGIN), 0, get_unique_color(), offset_index)
        car_paths[car_id] = []  # Initialize path as empty list
    
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
            return '', ''

        new_car_match = new_car_pattern.match(data)
        if new_car_match:
            new_car_id = new_car_match.group(1)
            return create_if_not_exists(new_car_id)

        path_match = path_pattern.match(data)
        if path_match:
            car_id = path_match.group(1)
            path_str = path_match.group(2)
            if car_id in car_data:
                process_path_message(car_id, path_str)
                return f'Path for Car ID {car_id} processed', 'white'
            return '', ''

        return data, 'white'  # Default color if no specific pattern matched

    return message, 'green'  # Color for non-matching messages

def process_path_message(car_id, path_str):
    """Process the path string and update the car's potential path."""
    if car_id in car_data:
        (x, y), orientation, color, offset_index = car_data[car_id]
        path = [(x, y + CELL_SIZE)]  # Start with the initial position

        for move in path_str:
            direction = int(move)

            if direction == 1:  # Turn right
                orientation = (orientation + 1) % 4
            elif direction == 2:  # Turn left
                orientation = (orientation - 1) % 4

            if orientation == 0:    # Facing up
                y -= CELL_SIZE
            elif orientation == 1:  # Facing right
                x += CELL_SIZE
            elif orientation == 2:  # Facing down
                y += CELL_SIZE
            elif orientation == 3:  # Facing left
                x -= CELL_SIZE

            path.append((x, y + CELL_SIZE))
        
        # Store the path
        car_paths[car_id] = path

def reset_data():
    """Reset all car data and paths but keep the grid."""
    global car_data, car_paths, available_colors
    car_data = {}  # Clear car data
    car_paths = {}  # Clear car paths
    available_colors = PREDEFINED_COLORS.copy()  # Reset available colors

    print("Resetting all car data and paths...")

def main():
    clock = pygame.time.Clock()
    stdin_fd = sys.stdin.fileno()

    while True:
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if RESET_BUTTON_RECT.collidepoint(event.pos):
                    reset_data()  # Call reset function

        draw_grid()

        # Use select to check for input availability
        readable, _, _ = select.select([stdin_fd], [], [], 0.1)
        if readable:
            message = sys.stdin.readline().strip()
            if message:
                processed_message, color = process_message(message)

                if color == 'white':
                    print(colored(processed_message, 'white'))
                elif color == 'green':
                    print(colored(processed_message, 'green'))

        draw_text_area()  # Draw the updated text area
        
        draw_cars()
        
        draw_paths()  # Ensure paths are drawn
        
        pygame.display.update()
        
        clock.tick(30)  # Limit to 30 frames per second

if __name__ == "__main__":
    main()

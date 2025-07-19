import sensor
import time
import math
from pyb import UART
import image


class Vector2:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.mag = math.sqrt(self.x*self.x + self.y*self.y)
        if self.mag == 0:
            self.mag = 1

    def normalize(self):
        if self.mag == 0:
            return Vector2(0, 0)
        return Vector2(self.x/self.mag, self.y/self.mag)

    def scale(self, factor):
        return Vector2(self.x*factor, self.y*factor)

    def as_tuple(self):
        return (int(self.x), int(self.y))

    def __add__(self, other):
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, other):
        return Vector2(self.x * other, self.y * other)

    def __truediv__(self, other):
        return Vector2(self.x / other, self.y / other)

    def __repr__(self):
        return f"Vector2({self.x}, {self.y})"

    def get_angle_degrees(self):
        if self.x == 0 and self.y == 0:
            return 0

        angle = math.atan2(-self.y, -self.x)

        angle_degrees = angle * 180 / math.pi

        return angle_degrees

    def rotate(self, degrees):
        radians = math.radians(degrees)

        old_x = self.x
        old_y = self.y

        new_x = old_x * math.cos(radians) - old_y * math.sin(radians)
        new_y = old_x * math.sin(radians) + old_y * math.cos(radians)

        return Vector2(new_x, new_y)


class Strategy:
    def __init__(self):
        self.state = "search"
        self.last_rotation = None
        # track rotation changes
        self.rotation_history = []
        self.frames_no_ball = 0
        self.frame = 0

    def update(self):
        # ball = get_ball()
        # diff = Vector2(0, 0)
        # if ball:
        #     diff = Vector2(ball.cx(), ball.cy()) - center
        #
        # angle = 0
        # goal = get_color_blob((0, 32, 5, 77, -47, -10))
        # if goal:
        #     angle = goal.cx() - center.x
        #
        # uart.write(f"V,{-diff.x / 100},0,{angle / 10}\n")
        # uart.write("R,40\n")
        # if self.state == "search":
        self.search()
        # elif self.state == "attack":
        # self.attack()
        # elif self.state == "recenter":
        #     self.recenter()
        # else:
        #     uart.write("S\n")

    def stay_in_bounds(self):
        movement = Vector2(0, 0)
        optimal_distance = 33
        for i in range(0, 360, 45):
            diff = raycaster(Vector2(-1, 0).rotate(i), field_threshold)

            if diff.mag < optimal_distance:
                img.draw_circle(*(center + diff).as_tuple(), 3, (255, 0, 0))
                movement += diff * (-diff.mag / (optimal_distance * 60))

        return movement

    def recenter(self):
        East = raycaster(Vector2(0, 1), field_threshold)
        West = raycaster(Vector2(0, -1), field_threshold)

        ball = get_ball()
        if ball:
            self.state = "search"

        angle = 0
        d_y = 164

        if abs(East.mag - West.mag) < 30:
            goal = get_goal()
            if goal:
                diff = Vector2(goal.cx(), goal.cy()) - center
                angle = diff.get_angle_degrees()
                d_y = diff.mag

        uart.write(f"V,{(East.mag - West.mag) /
                   90},{(d_y - 164) / 60},{angle}\n")

    def attack(self):
        ball = get_ball()
        goal = get_goal()
        goal_diff = Vector2(0, 0)

        angle = 0
        if goal:
            goal_diff = Vector2(goal.cx(), goal.cy()) - center
            if goal_diff.mag < 60:
                self.state = "search"
                uart.write("V,0,-0.4,0\n")
                return

        if ball:
            ball_diff = Vector2(ball.cx(), ball.cy()) - center
            angle = ball_diff.get_angle_degrees()
            if ball_diff.mag > 30:
                self.state = "search"

        uart.write(f"V,{-goal_diff.y / 100},{-goal_diff.x /
                   100},{angle + goal_diff.get_angle_degrees() / 3}\n")

        # ball = get_ball()
        # if ball:
        #     if not self.has_ball(ball):
        #         self.state = "search"
        #     # diff = Vector2(ball.cx(), ball.cy()) - center
        #     # if diff.mag > 40:
        #     #     self.state = "search"
        # else:
        #     self.state = "search"
        #
        # diff = self.stay_in_bounds()
        # uart.write(f"V,{diff.x},{diff.y},0\n")

        # goal = get_goal()
        # if goal:
        #     angle = 0
        #     goal_diff = Vector2(goal.cx(), goal.cy()) - center
        #
        #     if goal_diff.mag < 60:
        #         self.state = "search"
        #         uart.write(f"V,{-goal_diff.y / 80},{goal_diff.x / 80},0\n")
        #         return
        #
        #     angle = goal_diff.get_angle_degrees()
        #
        #     diff = Vector2(-70, 0)
        #
        #     ball = get_ball()
        #     if ball:
        #         ball_diff = Vector2(ball.cx(), goal.cy()) - center
        #         ball_diff.y = ball_diff.y * 1.5
        #         if ball_diff.x < 0:
        #             diff = ball_diff + Vector2(-50, 0)
        #         if ball_diff.mag > 30:
        #             self.state = "search"
        #
        #         # ball_angle = ball_diff.get_angle_degrees()
        #         # if abs(ball_diff.get_angle_degrees()) > 30:
        #         #     self.state = "search"
        #
        #         # if ball_angle > 15 or ball_angle < -15:
        #         #     self.state = "search"
        #         self.frames_no_ball = 0
        #     else:
        #         self.frames_no_ball += 1
        #         if self.frames_no_ball > 3:
        #             self.state = "search"
        #
        #     uart.write(f"V,{diff.y / 70},{diff.x / 70},{angle}\n")
        # else:
        #     # uart.write("S\n")
        #     uart.write("V,0,0.4,0\n")
        #     self.state = "search"

    def search(self):
        diff = Vector2(0, 0)
        angle = 0
        goal_diff = None

        goal = get_goal()
        if goal:
            pos = Vector2(goal.cx(), goal.cy())
            goal_diff = pos - center
            # img.draw_circle(*pos.as_tuple(), 5)
            # img.draw_circle(*pos.as_tuple(), 5)
            angle = (Vector2(goal.cx(), goal.cy()) -
                     center).get_angle_degrees()
            if goal_diff.mag < 100:
                uart.write(
                    f"V,{-goal_diff.y / 80},{goal_diff.x / 60},{angle}\n")
                return

        # else:
        #     our_goal = get_color_blob(our_goal_threshold)
        #     if our_goal:
        #         our_goal_diff = Vector2(our_goal.cx(), our_goal.cy()) - center
        #         angle = (our_goal_diff * -1).get_angle_degrees()

        ball = get_ball()

        if ball:
            self.frames_no_ball = 0
            ball_pos = Vector2(ball.cx(), ball.cy())
            # img.draw_circle(*ball_pos.as_tuple(), 5)
            diff = ball_pos - center
            ball_diff = ball_pos - center
            # ball_diff = ball_diff.rotate(-20)
            d_angle = abs(angle - diff.get_angle_degrees())
            # if diff.mag > 26:
            #     # diff = diff.rotate(math.asin(15 / diff.mag) * -57.2958)
            #     diff = diff.rotate(math.asin(26 / diff.mag) * -80.2958)
            if diff.mag > 15:
                if diff.get_angle_degrees() - angle < 0:
                    # diff = diff.rotate(math.asin(15 / diff.mag) * -90.2958)
                    diff = diff.rotate(math.asin(15 / diff.mag) * -120.2958)
                else:
                    diff = diff.rotate(math.asin(15 / diff.mag) * 120.2958)
                # diff = diff.rotate(math.asin(15 / diff.mag) * 100.2958)

                # diff = diff.rotate(math.asin(15 / diff.mag) * 90.2958)
            # else:
            #     diff = diff * -0.3
            # if d_angle > 10:
            #     diff *= 1.5
            # elif d_angle > 5:
            #     diff *= 1.3
            # elif d_angle < 5:
            #     self.state = "attack"
            # print(d_angle)

            # if ball_angle < 3 and ball_angle > -3:
            #     self.state = "attack"
            #     diff = Vector2(0, 0)
            # if diff.mag < 15 and abs(angle) < 20:
            #     self.state = "attack"
            #     diff = Vector2(0, 0)
            # if self.has_ball(ball) and abs(angle) < 20:
            #     diff.x -= 60
            # if self.has_ball(ball) and abs(angle) < 30:
            #     diff = Vector2(0, 0)

            # img.draw_circle(*(center + diff).as_tuple(), 5)

            ball_angle = (ball_pos - center).get_angle_degrees()
            # if goal_diff and (((ball_pos - center).x < -20) or (ball_angle > -15 and ball_angle < 15)):
            # if goal_diff and (ball_pos - center).x < -5:
            #     diff += goal_diff.normalize() * 60 / (ball_pos - center).mag
            if diff.mag < 20:
                self.state = "attack"

            if goal:
                ball_diff = ball_diff.rotate(-10)
                a = goal_diff.x * ball_diff.x + goal_diff.y * ball_diff.y
                if a > goal_diff.mag * 0.95 * ball_diff.mag:
                    diff += goal_diff * 0.7
            #     else:
            #         diff *= 1.2

        diff += self.stay_in_bounds() * 85

        uart.write(f"V,{diff.y / 60},{-diff.x / 60},{angle}\n")

        # else:
        #     uart.write("V,0.2,-0.2,8")

    def has_ball(self, ball):
        if ball:
            diff = Vector2(ball.cx(), ball.cy()) - center
            if abs(diff.get_angle_degrees()) < 15:
                return True
        else:
            return False


uart = UART(3, 115200)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

clock = time.clock()
center = Vector2(sensor.width() // 2, sensor.height() // 2)
center = Vector2(175, 109)

ball_threshold = (61, 86, 36, 127, -128, 127)
# ball_threshold = (61, 100, -1, 127, -3, 37)

goal_threshold = (44, 100, 1, 31, 19, 127)

field_threshold = (29, 100, -128, 10, -128, 46)

our_goal_threshold = (14, 33, -8, 127, -65, -10)


def get_goal():
    goal_blobs = img.find_blobs(
        [goal_threshold], area_threshold=1, pixels_threshold=1, merge=True)
    if len(goal_blobs) != 0:
        goal = max(goal_blobs, key=lambda obj: obj.pixels())
        return goal
    return None


def get_ball():
    ball_blobs = img.find_blobs(
        [ball_threshold], area_threshold=1, pixels_threshold=10, merge=True)
    if len(ball_blobs) != 0:
        ball = max(ball_blobs, key=lambda obj: obj.pixels())
        return ball
    return None


def get_color_blob(threshold):
    ball_blobs = img.find_blobs(
        [threshold], area_threshold=1, pixels_threshold=10, merge=True)
    if len(ball_blobs) != 0:
        ball = max(ball_blobs, key=lambda obj: obj.pixels())
        return ball
    return None


def is_col_at_point(pos, threshold):
    pixel = img.get_pixel(int(pos.x), int(pos.y))  # returns (r,g,b) tuple
    # convert rgb to lab
    if pixel:
        lab_col = image.rgb_to_lab((pixel[0], pixel[1], pixel[2]))
    else:
        return False

    # check color
    if (threshold[0] <= lab_col[0] <= threshold[1] and
        threshold[2] <= lab_col[1] <= threshold[3] and
            threshold[4] <= lab_col[2] <= threshold[5]):
        return True
    else:
        return False


def raycaster(diff, active_color):
    diff = diff.normalize() * 2
    pos = center + diff * 15
    for i in range(10, 100):
        # if is_col_at_point(pos, active_color) or is_col_at_point(pos, ball_threshold):
        if not is_col_at_point(pos, (0, 33, -9, 34, -15, 127)) and not is_col_at_point(pos, goal_threshold):
            pos += diff
        else:
            break

    return pos - center


strategy = Strategy()

while True:
    clock.tick()
    img = sensor.snapshot()
    img.lens_corr(1.8)
    img.draw_circle(*center.as_tuple(), 4)

    # img.draw_circle(*pos.as_tuple(), 5)

    strategy.update()

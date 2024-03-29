from dataclasses import dataclass

# event names
EVENT_CLICK = "Click"
EVENT_CURSOR = "CursorPosition"
EVENT_KEYSTROKE = "Keystroke"
EVENT_FIELD = "FieldCompletion"
EVENT_TASK = "TaskCompletion"


@dataclass
class Click:
    timestamp: int
    location: str
    was_correct: bool
    event_type: str = EVENT_CLICK

@dataclass
class CursorPosition:
    timestamp: int
    x: int
    y: int
    event_type: str = EVENT_CURSOR

@dataclass
class Keystroke:
    timestamp: int
    key: str
    was_correct: bool
    event_type: str = EVENT_KEYSTROKE

@dataclass
class FieldCompletion:
    timestamp: int
    field_index: int
    event_type: str = EVENT_FIELD

@dataclass
class TaskCompletion:
    timestamp: int
    task_index: int
    event_type: str = EVENT_TASK


def generate_statistics(events_flattened: list[dict]) -> dict:
    currTask = 0
    currField = 0

    lastFieldComplete = None
    lastClick = None
    # you'll want to iterate over this in a particular way
    # i think you can figure it out based on what i already typed up


def str_to_pybool(s: str) -> bool:
    return s == "true"

def main(filename: str) -> None:
    events = {
        EVENT_CLICK: [],
        EVENT_CURSOR: [],
        EVENT_KEYSTROKE: [],
        EVENT_FIELD: [],
        EVENT_TASK: []
    }

    with open(filename, "r") as logfile:
        for line in logfile:
            line = line.strip()
            if line == "":
                continue
            tokens = line.split(";")
            event_name = tokens[0]
            event_time = int(tokens[1])
            event_data = tokens[2:]

            if event_name == EVENT_CLICK:
                event = Click(event_time, event_data[0], str_to_pybool(event_data[1]))
            elif event_name == EVENT_CURSOR:
                event = CursorPosition(event_time, int(event_data[0]), int(event_data[1]))
            elif event_name == EVENT_KEYSTROKE:
                event = Keystroke(event_time, event_data[0], str_to_pybool(event_data[1]))
            elif event_name == EVENT_FIELD:
                event = FieldCompletion(event_time, int(event_data[0]))
            elif event_name == EVENT_TASK:
                event = TaskCompletion(event_time, int(event_data[0]))
            
            events[event_name].append(event)


    # sort events by timestamp
    events_chronological = []
    for _, value in events.items():
        events_chronological += value
    events_chronological.sort(key=lambda x: x.timestamp)

    # index is the index of the task
    # the inner lists are the events for that task
    task_events = [[], []]
    curr_task = 0
    for evt in events_chronological:
        if curr_task == 2:
            break
        
        if type(evt) == TaskCompletion:
            curr_task += 1
            continue
        
        task_events[curr_task].append(evt)

    import pprint
    pprint.pprint(task_events)
    # pprint.pprint(events, width=140)
    # pprint.pprint(events_chronological, width=140, sort_dicts=False)

    homing_times = []
    cursor_move_times = []
    field_wpms = []

    last_text_completion = None
    last_event_completion = None
    last_cursor = None
    for event in events_chronological:
        if event.event_type == EVENT_FIELD:
            last_text_completion = event
        elif event.event_type == EVENT_TASK:
            last_event_completion = event
        elif event.event_type == EVENT_CURSOR:
            if last_cursor is not None:
                delta_x = event.x - last_cursor.x
                delta_y = event.y - last_cursor.y
            

            if (last_text_completion is not None
                    and delta_x != 0
                    and delta_y != 0):
                homing_times.append(event.timestamp - last_text_completion.timestamp)

            last_cursor = event
    
    # import pprint
    # pprint.pprint(homing_times)
        


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python process.py <log filename>")
    else:
        filename = sys.argv[1]
        main(filename)

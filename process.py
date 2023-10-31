# event names
EVENT_CLICK = "Click"
EVENT_CURSOR = "CursorPosition"
EVENT_KEYSTROKE = "Keystroke"
EVENT_FIELD = "FieldCompletion"
EVENT_TASK = "TaskCompletion"


def make_click_event(tokens: list[str]) -> dict:
    return {
        "type": EVENT_CLICK,
        "timestamp": int(tokens[0]),
        "target": tokens[1],
        # assuming values are only {true | false}
        "correct": tokens[2] == "true"
    }


def make_cursor_event(tokens: list[str]) -> dict:
    return {
        "type": EVENT_CURSOR,
        "timestamp": int(tokens[0]),
        "xpos": int(tokens[1]),
        "ypos": int(tokens[2])
    }


def make_keystroke_event(tokens: list[str]) -> dict:
    return {
        "type": EVENT_KEYSTROKE,
        "timestamp": int(tokens[0]),
        "key": tokens[1],
        # assuming values are only {true | false}
        "correct": tokens[2] == "true"
    }


def make_field_event(tokens: list[str]) -> dict:
    return {
        "type": EVENT_FIELD,
        "timestamp": int(tokens[0]),
        "fieldIndex": int(tokens[1])
    }


def make_task_event(tokens: list[str]) -> dict:
    return {
        "type": EVENT_TASK,
        "timestamp": int(tokens[0])
        # task indices are never used anyway, all necessary info can reasonably be inferred from elsewhere
        # "taskIndex": int(tokens[1])
    }


def generate_statistics(events_flattened: list[dict]) -> dict:
    currTask = 0
    currField = 0

    lastFieldComplete = None
    lastClick = None
    # you'll want to iterate over this in a particular way
    # i think you can figure it out based on what i already typed up

handlers = {
    EVENT_CLICK: make_click_event,
    EVENT_CURSOR: make_cursor_event,
    EVENT_KEYSTROKE: make_keystroke_event,
    EVENT_FIELD: make_field_event,
    EVENT_TASK: make_task_event
}


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
            eventName = tokens[0]
            eventTokens = tokens[1:]

            evt = handlers[eventName](eventTokens)
            events[eventName].append(evt)

    # sort events by timestamp
    # probably not a big deal, they're most likely in order anyway
    comparator = lambda x: x["timestamp"]
    events_flattened = []
    for key, value in events.items():
        value.sort(key=comparator)
        for evt in value:
            events_flattened.append(evt)
    
    events_flattened.sort(key=comparator)


    import pprint
    pprint.pprint(events, width=140)
    pprint.pprint(events_flattened, width=140, sort_dicts=False)


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python process.py <log filename>")
    else:
        filename = sys.argv[1]
        main(filename)

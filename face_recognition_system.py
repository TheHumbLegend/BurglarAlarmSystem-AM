
#  Burglar Alarm – Face Recognition System
#  Continuously runs webcam but only scans for faces when
#  triggered by MATLAB (via command.txt).
#  Writes result to status.txt for MATLAB to poll.
#  Watches command.txt for add/remove/trigger commands.
#
# The facial recognition module was developed using the face_recognition
# library by Adam Geitgey [github.com/ageitgey/face_recognition] as a base.
# The implementation was extended to include continuous webcam streaming,
# a status file interface for MATLAB serial communication integration,
# and dynamic add/remove face functionality aligned with the system requirements.
#
#  Commands from MATLAB:
#    TRIGGER       → start scanning for faces
#    ADD:name      → capture and save new face
#    REMOVE:name   → delete face
#    QUIT          → shut down
#
#  Status written to status.txt:
#    IDLE              → not scanning
#    SCANNING          → triggered, looking for face
#    AUTHORIZED:name   → known face matched
#    UNAUTHORIZED      → unknown face detected
#    CAPTURING         → mid add-face flow
#    OFFLINE           → script stopped


import face_recognition
import cv2
import os
import numpy as np
import time

# ── Paths ────────────────────────────────────────────────
BASE_DIR        = os.path.dirname(os.path.abspath(__file__))
KNOWN_FACES_DIR = os.path.join(BASE_DIR, "known_faces")
STATUS_FILE     = os.path.join(BASE_DIR, "status.txt")
COMMAND_FILE    = os.path.join(BASE_DIR, "command.txt")

# ── Config ───────────────────────────────────────────────
TOLERANCE        = 0.5    # lower = stricter matching
FRAME_SKIP       = 3      # process every Nth frame
SCAN_TIMEOUT     = 10     # seconds to scan before giving up
UNKNOWN_LABEL    = "UNAUTHORIZED"
COOLDOWN_SECONDS = 15     # cooldown after any result before re-trigger


def write_status(status: str):
    """Write current recognition status to status.txt"""
    with open(STATUS_FILE, "w") as f:
        f.write(status)

def read_command() -> str:
    """Read and clear command.txt, returns empty string if no command"""
    if not os.path.exists(COMMAND_FILE):
        return ""
    try:
        with open(COMMAND_FILE, "r") as f:
            cmd = f.read().strip()
        with open(COMMAND_FILE, "w") as f:
            f.write("")
        return cmd
    except:
        return ""

def load_known_faces() -> tuple:
    """
    Load all face encodings from known_faces folder.
    Each subfolder = one person, named after them.
    Returns (encodings list, names list)
    """
    encodings = []
    names     = []

    if not os.path.exists(KNOWN_FACES_DIR):
        os.makedirs(KNOWN_FACES_DIR)
        print("[INFO] Created known_faces directory")
        return encodings, names

    for person_name in os.listdir(KNOWN_FACES_DIR):
        person_dir = os.path.join(KNOWN_FACES_DIR, person_name)
        if not os.path.isdir(person_dir):
            continue

        for filename in os.listdir(person_dir):
            if not filename.lower().endswith((".jpg", ".jpeg", ".png")):
                continue

            img_path = os.path.join(person_dir, filename)
            image    = face_recognition.load_image_file(img_path)
            encoding = face_recognition.face_encodings(image)

            if encoding:
                encodings.append(encoding[0])
                names.append(person_name)
                print(f"[INFO] Loaded face: {person_name} ({filename})")
            else:
                print(f"[WARN] No face found in {img_path}, skipping")

    print(f"[INFO] Total known faces loaded: {len(encodings)}")
    return encodings, names

def capture_and_save_face(name: str, known_encodings: list,
                          known_names: list) -> bool:
    """
    Opens webcam, captures a photo, saves to known_faces/name/
    Adds the new encoding to the running lists in place.
    Returns True if successful.
    """
    print(f"[CMD] Capturing face for: {name}")
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        print("[ERROR] Could not open webcam for capture")
        return False

    write_status("CAPTURING")
    captured = False

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        cv2.putText(frame, f"Adding: {name}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(frame, "Press SPACE to capture, ESC to cancel",
                    (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.6,
                    (255, 255, 255), 1)
        cv2.imshow("Add Face - Burglar Alarm", frame)

        key = cv2.waitKey(1)

        if key == 27:  # ESC - cancel
            print("[CMD] Face capture cancelled")
            break

        if key == 32:  # SPACE - capture
            person_dir = os.path.join(KNOWN_FACES_DIR, name)
            os.makedirs(person_dir, exist_ok=True)

            img_path = os.path.join(person_dir, f"{name}_1.jpg")
            cv2.imwrite(img_path, frame)

            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            encoding  = face_recognition.face_encodings(rgb_frame)

            if encoding:
                known_encodings.append(encoding[0])
                known_names.append(name)
                print(f"[CMD] Face added successfully: {name}")
                captured = True
            else:
                print("[WARN] No face detected in captured image, try again")
            break

    cap.release()
    cv2.destroyAllWindows()
    return captured

def remove_face(name: str, known_encodings: list,
                known_names: list) -> bool:
    """
    Removes a person's folder from known_faces and
    removes their encodings from the running lists.
    Returns True if successful.
    """
    person_dir = os.path.join(KNOWN_FACES_DIR, name)

    if not os.path.exists(person_dir):
        print(f"[CMD] Person not found: {name}")
        return False

    for f in os.listdir(person_dir):
        os.remove(os.path.join(person_dir, f))
    os.rmdir(person_dir)

    indices_to_remove = [i for i, n in enumerate(known_names) if n == name]
    for i in sorted(indices_to_remove, reverse=True):
        known_encodings.pop(i)
        known_names.pop(i)

    print(f"[CMD] Removed face: {name}")
    return True

# ── Main loop ────────────────────────────────────────────
def main():
    print("=" * 50)
    print("  Burglar Alarm – Face Recognition System")
    print("=" * 50)

    known_encodings, known_names = load_known_faces()

    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    if not cap.isOpened():
        print("[ERROR] Could not open webcam")
        return

    write_status("IDLE")
    print("[INFO] System running in IDLE mode")
    print("[INFO] Waiting for TRIGGER command from MATLAB...")
    print("[INFO] Commands: TRIGGER | ADD:name | REMOVE:name | QUIT")

    frame_count      = 0
    camera_triggered = False
    scan_start_time  = 0
    last_status      = "IDLE"
    authorized_until = 0
    cooldown_until   = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            print("[ERROR] Failed to grab frame")
            break

        frame_count += 1

        # ── Check for commands from MATLAB ──────────────
        cmd = read_command()
        if cmd:
            print(f"[CMD] Received: {cmd}")

            if cmd == "QUIT":
                print("[INFO] Quit command received, shutting down")
                break

            elif cmd == "TRIGGER":
                if time.time() > cooldown_until:
                    camera_triggered = True
                    scan_start_time  = time.time()
                    write_status("SCANNING")
                    last_status = "SCANNING"
                    print("[INFO] Camera triggered - scanning for faces...")
                else:
                    remaining = int(cooldown_until - time.time())
                    print(f"[INFO] Trigger ignored - cooldown active ({remaining}s remaining)")

            elif cmd.startswith("ADD:"):
                name = cmd.split(":", 1)[1].strip()
                cap.release()
                capture_and_save_face(name, known_encodings, known_names)
                cap = cv2.VideoCapture(0)
                cap.set(cv2.CAP_PROP_FRAME_WIDTH,  640)
                cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                write_status("IDLE")
                camera_triggered = False

            elif cmd.startswith("REMOVE:"):
                name = cmd.split(":", 1)[1].strip()
                remove_face(name, known_encodings, known_names)
                write_status("IDLE")

        # ── Scan timeout ────────────────────────────────
        if camera_triggered:
            elapsed = time.time() - scan_start_time
            if elapsed >= SCAN_TIMEOUT:
                print("[INFO] Scan timeout - no face detected")
                write_status(UNKNOWN_LABEL)
                last_status      = UNKNOWN_LABEL
                camera_triggered = False
                cooldown_until   = time.time() + COOLDOWN_SECONDS

        # ── Face recognition ────────────────────────────
        if camera_triggered and frame_count % FRAME_SKIP == 0:

            small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
            rgb_small   = cv2.cvtColor(small_frame, cv2.COLOR_BGR2RGB)

            face_locations = face_recognition.face_locations(rgb_small)
            face_encodings = face_recognition.face_encodings(
                rgb_small, face_locations)

            current_status = "SCANNING"

            for encoding in face_encodings:
                if not known_encodings:
                    current_status   = UNKNOWN_LABEL
                    camera_triggered = False
                    cooldown_until   = time.time() + COOLDOWN_SECONDS
                    break

                matches    = face_recognition.compare_faces(
                    known_encodings, encoding, TOLERANCE)
                distances  = face_recognition.face_distance(
                    known_encodings, encoding)
                best_match = np.argmin(distances)

                if matches[best_match]:
                    name             = known_names[best_match]
                    current_status   = f"AUTHORIZED:{name}"
                    authorized_until = time.time() + 3
                    camera_triggered = False
                    cooldown_until   = time.time() + COOLDOWN_SECONDS
                    print(f"[INFO] Cooldown started - {COOLDOWN_SECONDS}s")
                else:
                    current_status   = UNKNOWN_LABEL
                    camera_triggered = False
                    cooldown_until   = time.time() + COOLDOWN_SECONDS
                    print(f"[INFO] Cooldown started - {COOLDOWN_SECONDS}s")

            if current_status != last_status:
                write_status(current_status)
                print(f"[STATUS] {current_status}")
                last_status = current_status

        # ── Reset to IDLE after authorized_until expires ─
        if last_status.startswith("AUTHORIZED") and time.time() > authorized_until:
            write_status("IDLE")
            last_status = "IDLE"
            print("[INFO] Authorization expired - back to IDLE")

        # ── Status overlay on frame ─────────────────────
        display_frame = frame.copy()

        if camera_triggered or time.time() < authorized_until:
            small = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
            for (top, right, bottom, left) in \
                    face_recognition.face_locations(small):
                top    *= 4; right *= 4; bottom *= 4; left *= 4
                color   = (0, 255, 0) \
                    if "AUTHORIZED" in last_status else (0, 0, 255)
                cv2.rectangle(display_frame,
                              (left, top), (right, bottom), color, 2)
                cv2.putText(display_frame, last_status,
                            (left, top - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        # Cooldown indicator
        if time.time() < cooldown_until:
            remaining_cd = int(cooldown_until - time.time())
            cv2.putText(display_frame,
                        f"Cooldown: {remaining_cd}s",
                        (10, 70), cv2.FONT_HERSHEY_SIMPLEX,
                        0.5, (0, 165, 255), 1)

        status_color = (0, 255, 0)   if "AUTHORIZED"  in last_status \
                  else (0, 0, 255)   if "UNAUTHORIZED" in last_status \
                  else (0, 165, 255) if last_status == "SCANNING" \
                  else (200, 200, 200)

        cv2.putText(display_frame, f"Status: {last_status}",
                    (10, 20), cv2.FONT_HERSHEY_SIMPLEX,
                    0.6, status_color, 2)
        cv2.putText(display_frame,
                    f"Known: {len(set(known_names))} | "
                    f"Triggered: {camera_triggered}",
                    (10, 45), cv2.FONT_HERSHEY_SIMPLEX,
                    0.5, (200, 200, 200), 1)

        cv2.imshow("Burglar Alarm - Face Recognition", display_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    write_status("OFFLINE")
    print("[INFO] Face recognition system stopped")

if __name__ == "__main__":
    main()
# Calculate the hash of resource files for verification
import hashlib

def calc_file_hash(filepath):
    """Calculate the SHA256 hash of a file."""
    sha256 = hashlib.sha256()
    try:
        with open(filepath, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256.update(chunk)
        return sha256.hexdigest()
    except FileNotFoundError:
        print(f"File not found: {filepath}")
        return None

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python calcHash.py <file_path>")
    else:
        file_path = sys.argv[1]
        file_hash = calc_file_hash(file_path)
        if file_hash:
            print(f"SHA256 hash of '{file_path}':\n\n\t{file_hash}\n")
